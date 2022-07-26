#pragma once
#include <iostream>
#include <map>
#include <vector>

#define N 9

using namespace std;

struct DadInferenceResult
{
	int class_idx;
	float probability;
};

typedef struct _HIS_0_S_DriverActivity_T
{
	bool bDadDrinking = false;
	bool bDadEating = false;
	bool bDadPhoneInteraction = false;
	bool bDadSmoking = false;

	uint_fast8_t u8DadDrinkingConf = 0;
	uint_fast8_t u8DadEatingConf = 0;
	uint_fast8_t u8DadPhoneInteractionConf = 0;
	uint_fast8_t u8DadSmokingConf = 0;

} HIS_0_S_DriverActivity_T;

enum DriverActivitySort
{
	kSafeDriving = 0,
	kSmoking,
	kDrinking,
	kEating,
	kPhoneInteraction,
	kOtherActivity
};
struct Warning
{
	bool warning_status_;
	int warning_activity_;
	float warning_conf_;
};


struct TestActivityList
{
	int class_index_;
	int time_;

};


class DADFunction {
public:
	vector<map<int, float>> raw_result_buffer;
	int smooth_frame = 5;
	std::map<int, float> raw_result_;
	const float kConfThreshold = 0.6;
	DadInferenceResult inference_result_previous;
	Warning warning = { 0,0,0.0 };

	// Count
	int safe_driving_count = 0;
	int smoking_count = 0;
	int drinking_count = 0;
	int eating_count = 0;
	int phone_count = 0;
	int other_behavior_count = 0;
	int kFPS = 10;
	Warning previous_warning = { 0,0,0.0 };


	void InferenceResultMapping(const DadInferenceResult result, DadInferenceResult& mapping_result);
	void ThresholdFilter(const DadInferenceResult current_result,
		DadInferenceResult& filter_result);
	void ActivityCountInitial(const DadInferenceResult filter_result);
	void AlertLogicFunc(const DadInferenceResult filter_result);
	void DriverActivityDetectionAlertOutput(HIS_0_S_DriverActivity_T& Dad_OutputResult);
	void IntegrationFunctionLogic(HIS_0_S_DriverActivity_T& Dad_OutputResult);
	DadInferenceResult InferenceResultSmooth();
private:
};
