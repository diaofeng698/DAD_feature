#include "main.h"

void DADFunction::InferenceResultMapping(const DadInferenceResult result, DadInferenceResult &mapping_result)
{

	if (result.class_idx == 5)
	{
		mapping_result.class_idx = kPhoneInteraction;
	}
	else if (result.class_idx == 6 || result.class_idx == 7 || result.class_idx == 8)
	{
		mapping_result.class_idx = kSafeDriving;
	}
	else
	{
		mapping_result.class_idx = result.class_idx;
	}
	mapping_result.probability = result.probability;
	
	std::cout << "Mapping Result ID : " << mapping_result.class_idx << " Conf : " << mapping_result.probability << std::endl;
}

DadInferenceResult DADFunction::InferenceResultSmooth()
{
	//TODO:
	//std::map<int, float> raw_result = raw_result_;

	raw_result_buffer.push_back(raw_result_);
	map<int, float> sum;
	map<int, float> average_conf;
	if (raw_result_buffer.size() == smooth_frame + 1)
	{
		raw_result_buffer.assign(raw_result_buffer.begin() + 1, raw_result_buffer.end());

		for (vector<map<int, float>>::iterator it_raw_result_buffer = raw_result_buffer.begin();
			it_raw_result_buffer != raw_result_buffer.end(); it_raw_result_buffer++)
		{
			for (map<int, float>::iterator it_raw_result = it_raw_result_buffer->begin();
				it_raw_result != it_raw_result_buffer->end(); it_raw_result++)
			{
				sum[it_raw_result->first] += it_raw_result->second;
				average_conf[it_raw_result->first] = sum[it_raw_result->first] / raw_result_buffer.size();
			}
		}
	}
	else
	{
		for (vector<map<int, float>>::iterator it_raw_result_buffer = raw_result_buffer.begin();
			it_raw_result_buffer != raw_result_buffer.end(); it_raw_result_buffer++)
		{
			for (map<int, float>::iterator it_raw_result = it_raw_result_buffer->begin();
				it_raw_result != it_raw_result_buffer->end(); it_raw_result++)
			{
				sum[it_raw_result->first] += it_raw_result->second;
				average_conf[it_raw_result->first] = sum[it_raw_result->first] / raw_result_buffer.size();
			}
		}
	}
	std::cout << "Inference Result Smooth : " << std::endl;

	DadInferenceResult inference_result_smooth;
	inference_result_smooth.class_idx = 0;
	inference_result_smooth.probability = 0.0;

	for (std::map<int, float>::iterator it_average_conf = average_conf.begin(); it_average_conf != average_conf.end();
		it_average_conf++)
	{
		std::cout << it_average_conf->first << " : " << it_average_conf->second << std::endl;
		if (inference_result_smooth.probability < it_average_conf->second)
		{
			inference_result_smooth.probability = it_average_conf->second;
			inference_result_smooth.class_idx = it_average_conf->first;
		}
	}
	std::cout << "Inference Result Smooth Argmax" << std::endl;
	std::cout << "Inference Result ID : " << inference_result_smooth.class_idx
		<< " Conf : " << inference_result_smooth.probability << std::endl;


	return inference_result_smooth;


}

void DADFunction::ThresholdFilter(const DadInferenceResult current_result,
	DadInferenceResult &filter_result)
{

	if (current_result.probability >= kConfThreshold)
	{
		filter_result.class_idx = inference_result_previous.class_idx = current_result.class_idx;
		filter_result.probability = inference_result_previous.probability = current_result.probability;
		std::cout << "threshold filter ID : " << filter_result.class_idx << " Conf : " << filter_result.probability << std::endl;
	}
	else
	{
		filter_result.class_idx = inference_result_previous.class_idx;
		filter_result.probability = inference_result_previous.probability;
		std::cout << "threshold filter ID : " << filter_result.class_idx << " Conf : " << filter_result.probability << std::endl;
	}
}

void DADFunction::ActivityCountInitial(const DadInferenceResult filter_result)
{
	if (filter_result.class_idx == kSmoking)
	{
		drinking_count = eating_count = phone_count = safe_driving_count = other_behavior_count = 0;
		smoking_count++;
	}
	else if (filter_result.class_idx == kDrinking)
	{
		smoking_count = eating_count = phone_count = safe_driving_count = other_behavior_count = 0;
		drinking_count++;
	}
	else if (filter_result.class_idx == kEating)
	{
		smoking_count = drinking_count = phone_count = safe_driving_count = other_behavior_count = 0;
		eating_count++;
	}
	else if (filter_result.class_idx == kPhoneInteraction)
	{
		smoking_count = drinking_count = eating_count = safe_driving_count = other_behavior_count = 0;
		phone_count++;
	}
	else if (filter_result.class_idx == kSafeDriving)
	{
		smoking_count = drinking_count = eating_count = phone_count = other_behavior_count = 0;
		safe_driving_count++;
	}
	else if (filter_result.class_idx == kOtherActivity)
	{
		smoking_count = drinking_count = eating_count = phone_count = safe_driving_count = 0;
		other_behavior_count++;
	}
	std::cout << "***COUNT SUMMARY***" << std::endl;
	std::cout << "Safe: " << safe_driving_count << " Smoke: " << smoking_count << " Drink: " << drinking_count << " Eating: " << eating_count << " Phone: " << phone_count <<  " Other: " << other_behavior_count << std::endl;
}

void DADFunction::AlertLogicFunc(const DadInferenceResult filter_result)
{
	if (smoking_count >= 2 * kFPS)
	{
		warning.warning_status_ = 1;
		warning.warning_activity_ = kSmoking;
		warning.warning_conf_ = filter_result.probability;
	}
	else if (drinking_count >= 2 * kFPS)
	{
		warning.warning_status_ = 1;
		warning.warning_activity_ = kDrinking;
		warning.warning_conf_ = filter_result.probability;
	}
	else if (eating_count >= 2 * kFPS)
	{
		warning.warning_status_ = 1;
		warning.warning_activity_ = kEating;
		warning.warning_conf_ = filter_result.probability;
	}
	else if (phone_count >= 3 * kFPS)
	{
		warning.warning_status_ = 1;
		warning.warning_activity_ = kPhoneInteraction;
		warning.warning_conf_ = filter_result.probability;
	}
	else if (safe_driving_count >= 2 * kFPS)
	{
		warning.warning_status_ = 0;
	}
	else if (other_behavior_count >= 2 * kFPS)
	{
		warning.warning_status_ = 0;
	}
	else
	{
		if (previous_warning.warning_status_ == 1)
		{
			warning.warning_activity_ = previous_warning.warning_activity_;
			warning.warning_conf_ = previous_warning.warning_conf_;
		}
	}
	previous_warning.warning_activity_ = warning.warning_activity_;
	previous_warning.warning_conf_ = warning.warning_conf_;
	previous_warning.warning_status_ = warning.warning_status_;

	std::cout << "Alert Enable: " << warning.warning_status_ << " Alert Status: " << warning.warning_activity_ << " Alert Conf: " << warning.warning_conf_ << std::endl;
	
}

void DADFunction::DriverActivityDetectionAlertOutput(HIS_0_S_DriverActivity_T& Dad_OutputResult)
{
	Dad_OutputResult.bDadDrinking = false;
	Dad_OutputResult.bDadEating = false;
	Dad_OutputResult.bDadPhoneInteraction = false;
	Dad_OutputResult.bDadSmoking = false;
	Dad_OutputResult.u8DadDrinkingConf = 0;
	Dad_OutputResult.u8DadEatingConf = 0;
	Dad_OutputResult.u8DadPhoneInteractionConf = 0;
	Dad_OutputResult.u8DadSmokingConf = 0;

	if (warning.warning_status_ == true)
	{
		if (warning.warning_activity_ == kDrinking)
		{
			Dad_OutputResult.bDadDrinking = true;
			Dad_OutputResult.u8DadDrinkingConf = (uint_fast8_t)(warning.warning_conf_ * 100.0);
		}
		if (warning.warning_activity_ == kEating)
		{
			Dad_OutputResult.bDadEating = true;
			Dad_OutputResult.u8DadEatingConf = (uint_fast8_t)(warning.warning_conf_ * 100.0);
		}
		if (warning.warning_activity_ == kPhoneInteraction)
		{
			Dad_OutputResult.bDadPhoneInteraction = true;
			Dad_OutputResult.u8DadPhoneInteractionConf = (uint_fast8_t)(warning.warning_conf_ * 100.0);
		}
		if (warning.warning_activity_ == kSmoking)
		{
			Dad_OutputResult.bDadSmoking = true;
			Dad_OutputResult.u8DadSmokingConf = (uint_fast8_t)(warning.warning_conf_ * 100.0);
		}
	}
}

void DADFunction::IntegrationFunctionLogic(HIS_0_S_DriverActivity_T& Dad_OutputResult)
{
	DadInferenceResult inference_result_smooth;
	DadInferenceResult filter_result;
	DadInferenceResult mapping_result;
	
	inference_result_smooth = InferenceResultSmooth();
	ThresholdFilter(inference_result_smooth, filter_result);
	InferenceResultMapping(filter_result, mapping_result);
	ActivityCountInitial(mapping_result);
	AlertLogicFunc(mapping_result);
	//t_diff = (getMillisecondsTimeStamp() - t_global) / 1000.0;
	std::cout <<  " warning status(0:OFF,1:ON): " << warning.warning_status_ << std::endl;
	std::cout <<  " warning activity(0:SafeDriving,1:Smoking,2:Drinking,3:Eating,4:PhoneInteraction,5:OtherActivity): " << filter_result.class_idx << std::endl;
	DriverActivityDetectionAlertOutput(Dad_OutputResult);
}




int main()
{
	HIS_0_S_DriverActivity_T DadStateResult;
	DADFunction dad_func;


	DadInferenceResult inference_result_smooth;
	DadInferenceResult filter_result;
	DadInferenceResult mapping_result;
	
	srand((unsigned)time(NULL));
	
	// Test Module
	vector<TestActivityList> test_activity_list;
	TestActivityList temp_test_activity;

	// Activity 1
	temp_test_activity.class_index_ = kSafeDriving;
	temp_test_activity.time_ = 1;
	test_activity_list.push_back(temp_test_activity);

	// Activity 2
	temp_test_activity.class_index_ = kDrinking;
	temp_test_activity.time_ = 2;
	test_activity_list.push_back(temp_test_activity);

	vector<int> test_queue;
	for (vector<TestActivityList>::iterator it_test_activity_list = test_activity_list.begin(); it_test_activity_list != test_activity_list.end(); it_test_activity_list++)
	{
		for (int i = 0; i < it_test_activity_list->time_ * dad_func.kFPS; i++)
		{
			test_queue.push_back(it_test_activity_list->class_index_);
		}
	}


	int frame_index = 1;
	for (vector<int>::iterator it = test_queue.begin(); it != test_queue.end(); it++)
	{

		// TODO:  clear buffer
		dad_func.raw_result_.clear();
		// Insert rew result
		for (int i = 0; i < 9; i++) {
			float conf = rand() % (N + 1) / (float)(N + 1);
			// std::cout << conf << "   ";
			dad_func.raw_result_.insert(pair<int, float>(i, conf));
		}
		// std::cout << std::endl;
		// pipeline
		inference_result_smooth = dad_func.InferenceResultSmooth();
		dad_func.ThresholdFilter(inference_result_smooth, filter_result);
		dad_func.InferenceResultMapping(filter_result, mapping_result);
		//dad_func.ActivityCountInitial(mapping_result);
		//dad_func.AlertLogicFunc(mapping_result);


		cout << "****************Start*******************" << endl;
		cout << "current activity: " << *it << "    frame: " << frame_index << endl;
		DadInferenceResult test_mapping_result;
		test_mapping_result.class_idx = *it;
		test_mapping_result.probability = *it * 0.1;
		dad_func.ActivityCountInitial(test_mapping_result);
		dad_func.AlertLogicFunc(test_mapping_result);
		frame_index++;


	}
	
	
	
	//dad_func.IntegrationFunctionLogic(DadStateResult);
	system("pause");
    
    return 0;
}