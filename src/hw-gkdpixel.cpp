#include <string>
#include <vector>
#include <math.h>

#include "constants.h"
#include "hw-gkdpixel.h"
#include "hw-cpu.h"
#include "hw-power.h"
#include "hw-clock.h"
#include "hw-led.h"
#include "hw-hdmi.h"

#include "fileutils.h"

HwGkdPixel::HwGkdPixel() : IHardware() {
    TRACE("enter");

    this->INTERNAL_MOUNT_DEVICE = "/dev/mmcblk0";

    this->clock_ = (IClock *) new RTC();
    this->soundcard_ = (ISoundcard *) new AlsaSoundcard("default", "PCM");
    this->cpu_ = (ICpu *) new X1830Cpu();
    this->power_ = (IPower *)new JzPower();
    this->led_ = (ILed *)new DummyLed();
    this->hdmi_ = (IHdmi *)new Rg350Hdmi();

    this->pollBacklight = FileUtils::fileExists(BACKLIGHT_PATH);

    this->getBacklightLevel();
    this->getKeepAspectRatio();
    this->resetKeymap();

    TRACE(
        "brightness: %i, volume : %i",
        this->getBacklightLevel(),
        this->soundcard_->getVolume());
    TRACE("exit");
}

HwGkdPixel::~HwGkdPixel() {
    delete this->clock_;
    delete this->cpu_;
    delete this->soundcard_;
    delete this->power_;
    delete this->led_;
    delete this->hdmi_;
}

int HwGkdPixel::getBacklightLevel() {
    TRACE("enter");
    if (this->pollBacklight) {
        int level = 0;
        //force  scale 0 - 100
        std::string result = FileUtils::fileReader(BACKLIGHT_PATH);
        if (result.length() > 0) {
            level = ceil(atoi(StringUtils::trim(result).c_str()));
        }
        this->backlightLevel_ = level;
    }
    TRACE("exit : %i", this->backlightLevel_);
    return this->backlightLevel_;
}

int HwGkdPixel::setBacklightLevel(int val) {
    TRACE("enter - %i", val);
    // wrap it
    if (val <= 0)
        val = 100;
    else if (val > 100)
        val = 0;
    if (val == this->backlightLevel_)
        return val;

    if (FileUtils::fileWriter(BACKLIGHT_PATH, val)) {
        TRACE("success");
    } else {
        ERROR("Couldn't update backlight value to : %i", val);
    }
    this->backlightLevel_ = val;
    return this->backlightLevel_;
 }

bool HwGkdPixel::getKeepAspectRatio() { return true; }

bool HwGkdPixel::setKeepAspectRatio(bool val) { return val; }

std::string HwGkdPixel::getDeviceType() { return "GKD Pixel"; }

bool HwGkdPixel::setScreenState(const bool &enable) {
    TRACE("enter : %s", (enable ? "on" : "off"));
    const char *path = SCREEN_BLANK_PATH.c_str();
    const char *blank = enable ? "0" : "1";
    return this->writeValueToFile(path, blank);
}

void HwGkdPixel::resetKeymap() {
    this->writeValueToFile(ALT_KEYMAP_FILE, "N");
}
