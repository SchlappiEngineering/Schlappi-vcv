//
// Created by arden on 5/4/25.
//

#ifndef SCHLAPPI_VCV_SCHLAPPI_WIDGETS_H
#define SCHLAPPI_VCV_SCHLAPPI_WIDGETS_H

#include "plugin.hpp"

struct SchlappiToggleVertical2pos : rack::app::SvgSwitch {
    SchlappiToggleVertical2pos() {
        addFrame(APP->window->loadSvg(rack::asset::plugin(pluginInstance, "res/widgets/toggle-0.svg")));
        addFrame(APP->window->loadSvg(rack::asset::plugin(pluginInstance, "res/widgets/toggle-1.svg")));
    }
};

struct SchlappiCherryMXBrown : rack::app::SvgSwitch {
    SchlappiCherryMXBrown() {
        momentary = true;
        addFrame(APP->window->loadSvg(rack::asset::plugin(pluginInstance, "res/widgets/keyboard-button-0.svg")));
        addFrame(APP->window->loadSvg(rack::asset::plugin(pluginInstance, "res/widgets/keyboard-button-1.svg")));
    }
};

struct SchlappiSilverKnob : RoundKnob {
    SchlappiSilverKnob() {
        setSvg(APP->window->loadSvg(rack::asset::plugin(pluginInstance, "res/widgets/silver-knob-dark.svg")));
        bg->setSvg(APP->window->loadSvg(rack::asset::plugin(pluginInstance, "res/widgets/silver-knob-bg-dark.svg")));
    }
};

#endif //SCHLAPPI_VCV_SCHLAPPI_WIDGETS_H
