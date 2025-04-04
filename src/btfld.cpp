#include "plugin.hpp"


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
		OUT8_OUTPUT,
		OUT4_OUTPUT,
		OUT2_OUTPUT,
		STEP_OUT_OUTPUT,
		OUT1_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LEVEL_7_LIGHT,
		LEVEL_6_LIGHT,
		LEVEL_5_LIGHT,
		LEVEL_4_LIGHT,
		LEVEL_3_LIGHT,
		LEVEL_2_LIGHT,
		LEVEL_1_LIGHT,
		LEVEL_0_LIGHT,
		SAW_INDICATOR_LIGHT,
		INPUT_INDICATOR_LIGHT,
		INDICATOR_8_LIGHT,
		CV_INDICATOR_LIGHT,
		INDICATOR_4_LIGHT,
		INJECT_INDICATOR_LIGHT,
		INDICATOR_2_LIGHT,
		STEP_INDICATOR_LIGHT,
		INDICATOR_1_LIGHT,
		LIGHTS_LEN
	};

	Btfld() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(GAIN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RANGE_PARAM, 0.f, 1.f, 0.f, "");
		configInput(INPUT_INPUT, "");
		configInput(CV_INPUT, "");
		configInput(INJECT_INPUT, "");
		configOutput(SAW_OUTPUT, "");
		configOutput(OUT8_OUTPUT, "");
		configOutput(OUT4_OUTPUT, "");
		configOutput(OUT2_OUTPUT, "");
		configOutput(STEP_OUT_OUTPUT, "");
		configOutput(OUT1_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
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
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.541, 72.886)), module, Btfld::OUT8_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.541, 85.892)), module, Btfld::OUT4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.539, 98.809)), module, Btfld::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.196, 111.736)), module, Btfld::STEP_OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.541, 111.736)), module, Btfld::OUT1_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.247, 11.655)), module, Btfld::LEVEL_7_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.247, 16.659)), module, Btfld::LEVEL_6_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.247, 21.663)), module, Btfld::LEVEL_5_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.247, 26.646)), module, Btfld::LEVEL_4_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.247, 31.629)), module, Btfld::LEVEL_3_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.247, 36.654)), module, Btfld::LEVEL_2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.247, 41.637)), module, Btfld::LEVEL_1_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.247, 46.62)), module, Btfld::LEVEL_0_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(27.213, 53.376)), module, Btfld::SAW_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(13.868, 66.552)), module, Btfld::INPUT_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(27.213, 66.552)), module, Btfld::INDICATOR_8_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(13.868, 79.389)), module, Btfld::CV_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(27.213, 79.389)), module, Btfld::INDICATOR_4_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(13.868, 92.543)), module, Btfld::INJECT_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(27.213, 92.543)), module, Btfld::INDICATOR_2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(13.868, 105.232)), module, Btfld::STEP_INDICATOR_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(27.213, 105.232)), module, Btfld::INDICATOR_1_LIGHT));
	}
};


Model* modelBtfld = createModel<Btfld, BtfldWidget>("btfld");