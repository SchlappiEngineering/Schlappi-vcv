#include "plugin.hpp"
#include <iostream>
#include <cmath>

struct ACCouplingFilter {
    ACCouplingFilter() : xPrev(0), yPrev(0), scalar(0) {}

    void setDecay(float halflife) {
        scalar = std::pow(2, -1.f / halflife);
    }

    float process(float x) {
        auto y = scalar * (x + yPrev - xPrev);
        yPrev = y;
        xPrev = x;
        return y;
    }
public:
    float xPrev, yPrev, scalar;
};

float saturate(float x) {
    if ((-9 <= x) && (x <= 9)) {
        return x;
    } else if (x < -9) {
        return std::tanh(x + 9) - 9;
    } else {
        return std::tanh(x - 9) + 9;
    }
}

struct Btfld : Module {
	enum ParamId {
		GAIN_PARAM,
		CV_PARAM,
		RANGE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INPUT_INPUT,
		CV_INPUT,
		INJECT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		SAW_OUTPUT,
        ENUMS(BIT_OUTPUT, 4),
        STEP_OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
        ENUMS(LEVEL_LIGHT, 8),
		SAW_INDICATOR_LIGHT,
		INPUT_INDICATOR_LIGHT,
		CV_INDICATOR_LIGHT,
        INJECT_INDICATOR_LIGHT,
        ENUMS(BIT_INDICATOR_LIGHT, 4),
        STEP_INDICATOR_LIGHT,
		LIGHTS_LEN
	};

    float feedback;
    ACCouplingFilter stepFilter;
    ACCouplingFilter sawFilter;

	Btfld() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(GAIN_PARAM, 0.f, 2.f, 1.f, "Gain");
		configParam(CV_PARAM, 0.f, 1.f, 0.f, "Gain CV");
		configParam(RANGE_PARAM, 0.f, 1.f, 0.f, "Range (+10/±5)");
		configInput(INPUT_INPUT, "In");
		configInput(CV_INPUT, "CV");
		configInput(INJECT_INPUT, "Inject");
		configOutput(SAW_OUTPUT, "Saw");
		configOutput(BIT_OUTPUT + 3, "Out 8");
		configOutput(BIT_OUTPUT + 2, "Out 4");
		configOutput(BIT_OUTPUT + 1, "Out 2");
        configOutput(BIT_OUTPUT, "Out 1");
        configOutput(STEP_OUT_OUTPUT, "Step");

        feedback = 0;
	}

    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        stepFilter.setDecay(0.25f * e.sampleRate);
        sawFilter.setDecay(0.25f * e.sampleRate);
    }

    void process(const ProcessArgs& args) override {
        auto cvInput = inputs[CV_INPUT].isConnected() ? inputs[CV_INPUT].getVoltage() : feedback;
        auto gain = params[GAIN_PARAM].getValue() + params[CV_PARAM].getValue() * cvInput * 0.1;

        auto inputSignal = inputs[INPUT_INPUT].getVoltage();
        auto bipolar = params[RANGE_PARAM].getValue() > 0.5f;
        inputSignal *= gain;
        inputSignal += bipolar ? 5.f : 0.f;

        inputSignal += inputs[INJECT_INPUT].getVoltage();

        inputSignal = std::max(inputSignal, 0.f);

        inputSignal = std::min(inputSignal, 11.7f); // TODO: more natural saturation

        auto steps = static_cast<int>(floor(inputSignal * (15.f / 10.f)));

        steps = std::max(0, std::min(steps, 15));

        for (auto i = 0; i < 4; ++i) {
            if (steps & (1 << i)) {
                outputs[BIT_OUTPUT + i].setVoltage(bipolar ? 5.f : 10.f);
                lights[BIT_INDICATOR_LIGHT + i].setBrightnessSmooth(1.f, args.sampleTime);
            } else {
                outputs[BIT_OUTPUT + i].setVoltage(bipolar ? -5.f : 0.f);
                lights[BIT_INDICATOR_LIGHT + i].setBrightnessSmooth(0.f, args.sampleTime);
            }
        }

        auto rescaledSteps = static_cast<float>(steps) * (10.f / 15.f);
        auto filteredSteps = stepFilter.process(rescaledSteps);
        outputs[STEP_OUT_OUTPUT].setVoltage(saturate(bipolar ? filteredSteps : rescaledSteps));
        auto saw = inputSignal - rescaledSteps;
        auto filteredSaw = sawFilter.process(saw);
        feedback = saturate((bipolar ? filteredSaw : saw) * 10.f);
        outputs[SAW_OUTPUT].setVoltage(feedback);
    }
};


struct BtfldWidget : ModuleWidget {
	BtfldWidget(Btfld* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/btfld.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.22, 16.406)), module, Btfld::GAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.22, 41.785)), module, Btfld::CV_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(7.217, 58.613)), module, Btfld::RANGE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.196, 72.886)), module, Btfld::INPUT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.196, 85.892)), module, Btfld::CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.196, 98.729)), module, Btfld::INJECT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.541, 60.048)), module, Btfld::SAW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.541, 72.886)), module, Btfld::BIT_OUTPUT + 3));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.541, 85.892)), module, Btfld::BIT_OUTPUT + 2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.539, 98.809)), module, Btfld::BIT_OUTPUT + 1));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.541, 111.736)), module, Btfld::BIT_OUTPUT + 0));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.196, 111.736)), module, Btfld::STEP_OUT_OUTPUT));

		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 11.655)), module, Btfld::LEVEL_LIGHT + 7));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 16.659)), module, Btfld::LEVEL_LIGHT + 6));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 21.663)), module, Btfld::LEVEL_LIGHT + 5));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 26.646)), module, Btfld::LEVEL_LIGHT + 4));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 31.629)), module, Btfld::LEVEL_LIGHT + 3));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 36.654)), module, Btfld::LEVEL_LIGHT + 2));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 41.637)), module, Btfld::LEVEL_LIGHT + 1));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(3.247, 46.62)), module, Btfld::LEVEL_LIGHT + 0));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(27.213, 53.376)), module, Btfld::SAW_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(13.868, 66.552)), module, Btfld::INPUT_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(27.213, 66.552)), module, Btfld::BIT_INDICATOR_LIGHT + 3));
        addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(27.213, 79.389)), module, Btfld::BIT_INDICATOR_LIGHT + 2));
        addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(27.213, 92.543)), module, Btfld::BIT_INDICATOR_LIGHT + 1));
        addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(27.213, 105.232)), module, Btfld::BIT_INDICATOR_LIGHT + 0));
        addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(13.868, 79.389)), module, Btfld::CV_INDICATOR_LIGHT));
        addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(13.868, 92.543)), module, Btfld::INJECT_INDICATOR_LIGHT));
        addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(13.868, 105.232)), module, Btfld::STEP_INDICATOR_LIGHT));
	}
};


Model* modelBtfld = createModel<Btfld, BtfldWidget>("btfld");