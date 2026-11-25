#include <IOKit/IOLib.h>
#include <IOKit/IORegistryEntry.h>
#include <IOKit/IOService.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <libkern/c++/OSData.h>
#include <libkern/c++/OSNumber.h>
#include <libkern/c++/OSObject.h>
#include <mach/kmod.h>
#include <mach/mach_types.h>
#include <sys/sysctl.h>

class LegionECMode : public IOService {
    OSDeclareDefaultStructors(LegionECMode)

public:
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
};

#define super IOService
OSDefineMetaClassAndStructors(LegionECMode, IOService)

static IOLock *gLock = nullptr;
static IOACPIPlatformDevice *gGzfd = nullptr;
static int gModeVal = 2;

static IOACPIPlatformDevice *getGzfdDevice() {
    if (gGzfd) {
        return gGzfd;
    }
    IORegistryEntry *entry = IORegistryEntry::fromPath("IOACPIPlane:/_SB/GZFD");
    if (!entry) {
        return nullptr;
    }
    IOACPIPlatformDevice *dev = OSDynamicCast(IOACPIPlatformDevice, entry);
    if (!dev) {
        entry->release();
        return nullptr;
    }
    gGzfd = dev; // retained by fromPath
    return gGzfd;
}

static bool callWmaa(uint32_t methodId, uint32_t argVal, uint32_t *outNum) {
    IOACPIPlatformDevice *dev = getGzfdDevice();
    if (!dev) {
        return false;
    }

    OSNumber *arg0 = OSNumber::withNumber((uint64_t)0, 32);
    OSNumber *arg1 = OSNumber::withNumber((uint64_t)methodId, 32);
    OSData *arg2 = OSData::withBytes(&argVal, sizeof(argVal));
    OSObject *result = nullptr;
    bool ok = false;

    if (arg0 && arg1 && arg2) {
        OSObject *params[3] = {arg0, arg1, arg2};
        IOReturn ret = dev->evaluateObject("WMAA", &result, params, 3);
        if (ret == kIOReturnSuccess) {
            ok = true;
            if (outNum && result) {
                OSNumber *num = OSDynamicCast(OSNumber, result);
                if (num) {
                    *outNum = num->unsigned32BitValue();
                }
            }
        }
    }

    if (result) result->release();
    if (arg0) arg0->release();
    if (arg1) arg1->release();
    if (arg2) arg2->release();
    return ok;
}

static int sysctlModeHandler(struct sysctl_oid *oidp, void *arg1, int arg2, struct sysctl_req *req) {
    int val = gModeVal;

    if (!req->newptr) {
        if (gLock) IOLockLock(gLock);
        uint32_t hwMode = 0;
        if (callWmaa(0x2D, 0, &hwMode)) {
            val = (int)hwMode;
            gModeVal = val;
        }
        if (gLock) IOLockUnlock(gLock);
    }

    int err = sysctl_handle_int(oidp, &val, 0, req);
    if (err || !req->newptr) {
        return err;
    }

    // Only allow Quiet (1), Balanced (2), Performance (3)
    if (val < 1 || val > 3) {
        return EINVAL;
    }

    if (gLock) IOLockLock(gLock);
    bool ok = callWmaa(0x2C, (uint32_t)val, nullptr);
    if (ok) {
        gModeVal = val;
    }
    if (gLock) IOLockUnlock(gLock);

    return ok ? 0 : EIO;
}

SYSCTL_NODE(_debug, OID_AUTO, legionec, CTLFLAG_RW | CTLFLAG_LOCKED, 0, "Legion EC power mode control");
SYSCTL_PROC(_debug_legionec, OID_AUTO, mode,
            CTLTYPE_INT | CTLFLAG_RW | CTLFLAG_ANYBODY | CTLFLAG_LOCKED,
            &gModeVal, 0, sysctlModeHandler, "I",
            "Legion EC SmartFan mode (1=Quiet, 2=Balanced, 3=Performance)");

bool LegionECMode::start(IOService *provider) {
    if (!super::start(provider)) {
        return false;
    }
    if (!gLock) {
        gLock = IOLockAlloc();
    }
    sysctl_register_oid(&sysctl__debug_legionec);
    sysctl_register_oid(&sysctl__debug_legionec_mode);
    registerService();
    IOLog("LegionECMode: started\n");
    return true;
}

void LegionECMode::stop(IOService *provider) {
    sysctl_unregister_oid(&sysctl__debug_legionec_mode);
    sysctl_unregister_oid(&sysctl__debug_legionec);
    if (gLock) {
        IOLockLock(gLock);
        if (gGzfd) {
            gGzfd->release();
            gGzfd = nullptr;
        }
        IOLockUnlock(gLock);
        IOLockFree(gLock);
        gLock = nullptr;
    }
    super::stop(provider);
}

extern "C" {
kern_return_t _start(kmod_info_t *ki, void *data);
kern_return_t _stop(kmod_info_t *ki, void *data);
kern_return_t (*_realmain)(kmod_info_t *ki, void *data) = nullptr;
kern_return_t (*_antimain)(kmod_info_t *ki, void *data) = nullptr;
__attribute__((visibility("default")))
KMOD_EXPLICIT_DECL(org.kslengyu.LegionECMode, "1.0.0", _start, _stop)
}
