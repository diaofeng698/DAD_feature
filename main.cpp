#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>

using namespace std;

struct InferenceResult
{
	int state_now_;  
	float conf_;    
}; 


struct CountInferenceResult
{
	int count_;
	float conf_;
};

enum DriverActivitySort
{
	kSafeDriving = 0,
	kSmoking,
	kDrinking,
	kEating,
	kPhoneInteraction,
	kOtherActivity
};


struct TestActivityList
{
	int class_index_;
	int time_;

};

struct Warning
{
	bool warning_status_;
	int warning_activity_;
};

 




void SortBuffer(map<int, CountInferenceResult> multi_activity_buffer, int* alert_result, int* longest_frame, float* alert_conf )
{
	for (map<int, CountInferenceResult>::iterator it_multi_activity_buffer = multi_activity_buffer.begin(); it_multi_activity_buffer != multi_activity_buffer.end(); it_multi_activity_buffer++)
	{
		if (it_multi_activity_buffer->second.count_ > *longest_frame)
		{
			*longest_frame = it_multi_activity_buffer->second.count_;
			*alert_result = it_multi_activity_buffer->first;
			*alert_conf = it_multi_activity_buffer->second.conf_;
		}
		else if(it_multi_activity_buffer->second.count_ == *longest_frame)
		{
			int item_index = it_multi_activity_buffer->first;
			int longest_frame_index = *alert_result;
			if (item_index > longest_frame_index)
			{
				*alert_result = it_multi_activity_buffer->first;
				*alert_conf = it_multi_activity_buffer->second.conf_;
			}

		}
	}


	cout << "alert_result " << *alert_result << " longest_frame " << *longest_frame << " alert_conf " << *alert_conf << endl;

}


bool OutputAlert(const int alert_result, const int longest_frame, const float alert_conf)
{
	float average_alert_conf = alert_conf / longest_frame;
	bool warning_status = true;
	cout << "alert: " << alert_result << " conf : " << average_alert_conf << " last longest time in 50 frames" << endl;
	return warning_status;
}



int main()
{

	// Param
	const int kResetTime = 2;
	const int KAlertTime = 10;
	const int kBufferTime = 30;
	cout << "reset time range is " << kResetTime <<" s, alert time range is "<< KAlertTime << " s, buffer time range is " << kBufferTime << " s" << endl;
	const float kConfThreshold = 0.0;
	const int kSafeMode = 0;


	// For Frame
	const int kFPS = 5;
	int reset_frame = kResetTime * kFPS;
	int alert_frame = KAlertTime * kFPS;
	int buffer_frame = kBufferTime * kFPS;


	// inference result now
	int frame_now;
	float conf;

	int state_previous = 0;

	//Input
	vector<InferenceResult> buffer_list;
	int safe_mode_buffer;

	// Output
	Warning warning;
	warning.warning_status_ = false;


	//For test
	vector<TestActivityList> test_activity_list;
	TestActivityList temp_test_activity;

	// Activity 1
	temp_test_activity.class_index_ = kEating;
	temp_test_activity.time_ = 5;
	test_activity_list.push_back(temp_test_activity);

	// Activity 2
	temp_test_activity.class_index_ = kSafeDriving;
	temp_test_activity.time_ = 2;
	test_activity_list.push_back(temp_test_activity);

	// Activity 3
	temp_test_activity.class_index_ = kDrinking;
	temp_test_activity.time_ = 5;
	test_activity_list.push_back(temp_test_activity);

	// Activity 4
	temp_test_activity.class_index_ = kSafeDriving;
	temp_test_activity.time_ = 3;
	test_activity_list.push_back(temp_test_activity);

	// Activity 5
	temp_test_activity.class_index_ = kSmoking;
	temp_test_activity.time_ = 5;
	test_activity_list.push_back(temp_test_activity);

	// Activity 6
	temp_test_activity.class_index_ = kPhoneInteraction;
	temp_test_activity.time_ = 5;
	test_activity_list.push_back(temp_test_activity);

	// Activity 7
	temp_test_activity.class_index_ = kSafeDriving;
	temp_test_activity.time_ = 1;
	test_activity_list.push_back(temp_test_activity);

	// Activity 8
	temp_test_activity.class_index_ = kDrinking;
	temp_test_activity.time_ = 4;
	test_activity_list.push_back(temp_test_activity);

	// Activity 9
	temp_test_activity.class_index_ = kEating;
	temp_test_activity.time_ = 10;
	test_activity_list.push_back(temp_test_activity);

	vector<int> test_queue;
	for (vector<TestActivityList>::iterator it_test_activity_list = test_activity_list.begin(); it_test_activity_list != test_activity_list.end(); it_test_activity_list++)
	{
		for (int i = 0; i < it_test_activity_list->time_*kFPS; i++) 
		{
			test_queue.push_back(it_test_activity_list->class_index_);
		}
	}

	
	int frame_index = 1;
	for (vector<int>::iterator it = test_queue.begin(); it != test_queue.end(); it++)
	{
		cout << "current activity: " << *it << "    frame: " << frame_index << endl;
		frame_now = *it;
		conf = 1.0;

		//For test

		InferenceResult temp_inference_result;
		temp_inference_result.state_now_ = frame_now;
		temp_inference_result.conf_ = conf;


		if (conf >= kConfThreshold)
		{
			//cout << "Start" << endl;

			//FIFO
			buffer_list.push_back(temp_inference_result);
			if (buffer_list.size() == buffer_frame + 1)
			{
				buffer_list.assign(buffer_list.begin() + 1, buffer_list.end());
			}

			//Sum and count
			map<int, CountInferenceResult> buffer;
			for (vector<InferenceResult>::iterator it_buffer_list = buffer_list.begin(); it_buffer_list != buffer_list.end(); it_buffer_list++)
			{
				if (!buffer.count(it_buffer_list->state_now_))
				{
					buffer[it_buffer_list->state_now_] = CountInferenceResult{ 1, it_buffer_list->conf_ };
				}
				else
				{
					buffer[it_buffer_list->state_now_].count_ += 1;
					buffer[it_buffer_list->state_now_].conf_ += conf;
				}

			}

			if (temp_inference_result.state_now_ != kSafeMode)
			{
				safe_mode_buffer = 0;

				// multi_activity_buffer
				map<int, CountInferenceResult> multi_activity_buffer;
				for (map<int, CountInferenceResult>::iterator it_buffer = buffer.begin(); it_buffer != buffer.end(); it_buffer++)
				{
					if (it_buffer->first == kSmoking || it_buffer->first == kDrinking || it_buffer->first == kEating || it_buffer->first == kPhoneInteraction)
					{
						multi_activity_buffer[it_buffer->first] = it_buffer->second;
					}
				}

				// max time 
				int max_time = 0;
				int total_time = 0;
				if (multi_activity_buffer.size() != 0)
				{
					vector<int> time_array_max;
					for (map<int, CountInferenceResult>::iterator it_multi_activity_buffer = multi_activity_buffer.begin(); it_multi_activity_buffer != multi_activity_buffer.end(); it_multi_activity_buffer++)
					{
						time_array_max.push_back(it_multi_activity_buffer->second.count_);
					}
					max_time = *max_element(time_array_max.begin(), time_array_max.end());
				}

				if (max_time >= alert_frame)
				{
					cout << "max_time: " << max_time << endl;
					int alert_result = 1;
					int longest_frame = 0;
					float alert_conf = 0.0;
					SortBuffer(multi_activity_buffer, &alert_result, &longest_frame, &alert_conf);

					cout << "-----> single activity: " << alert_result << endl;
					warning.warning_status_ = OutputAlert(alert_result, longest_frame, alert_conf);
					warning.warning_activity_ = alert_result;
				}
				else
				{
					// Sum
					vector<int> time_array_sum;
					for (map<int, CountInferenceResult>::iterator it_multi_activity_buffer = multi_activity_buffer.begin(); it_multi_activity_buffer != multi_activity_buffer.end(); it_multi_activity_buffer++)
					{
						time_array_sum.push_back(it_multi_activity_buffer->second.count_);
					}
					total_time = accumulate(time_array_sum.begin(), time_array_sum.end(), 0);

					if (total_time >= alert_frame)
					{
						cout << "total_time: " << total_time << endl;
						int alert_result = 1;
						int longest_frame = 0;
						float alert_conf = 0.0;
						SortBuffer(multi_activity_buffer, &alert_result, &longest_frame, &alert_conf);

						cout << "-----> multi  activity: " << alert_result << endl;
						warning.warning_status_ = OutputAlert(alert_result, longest_frame, alert_conf);
						warning.warning_activity_ = alert_result;

					}

				}


			}


			else
			{
				//Safe Driving Time Accumulate
				if (state_previous == kSafeMode)
				{
					safe_mode_buffer += 1;
				}
				else
				{
					safe_mode_buffer = 1;
				}


				if (safe_mode_buffer >= reset_frame)
				{
					if (warning.warning_status_ == true)
					{
						warning.warning_status_ = false;
						cout << "safe driving mode last for 10 frames, release warning" << endl;
					}
					safe_mode_buffer -= 1;

				}
				else
				{
					if (warning.warning_status_ == true)
					{

						// multi_activity_buffer
						map<int, CountInferenceResult> multi_activity_buffer;
						for (map<int, CountInferenceResult>::iterator it_buffer = buffer.begin(); it_buffer != buffer.end(); it_buffer++)
						{
							if (it_buffer->first == kSmoking || it_buffer->first == kDrinking || it_buffer->first == kEating || it_buffer->first == kPhoneInteraction)
							{
								multi_activity_buffer[it_buffer->first] = it_buffer->second;
							}
						}

						// max time 
						int max_time = 0;
						int total_time = 0;
						if (multi_activity_buffer.size() != 0)
						{
							vector<int> time_array_max;
							for (map<int, CountInferenceResult>::iterator it_multi_activity_buffer = multi_activity_buffer.begin(); it_multi_activity_buffer != multi_activity_buffer.end(); it_multi_activity_buffer++)
							{
								time_array_max.push_back(it_multi_activity_buffer->second.count_);
							}
							max_time = *max_element(time_array_max.begin(), time_array_max.end());
						}

						if (max_time >= alert_frame)
						{
							cout << "max_time: " << max_time << endl;
							int alert_result = 1;
							int longest_frame = 0;
							float alert_conf = 0.0;
							SortBuffer(multi_activity_buffer, &alert_result, &longest_frame, &alert_conf);

							cout << "-----> single activity: " << alert_result << endl;
							warning.warning_status_ = OutputAlert(alert_result, longest_frame, alert_conf);
							warning.warning_activity_ = alert_result;
						}
						else
						{
							// Sum
							vector<int> time_array_sum;
							for (map<int, CountInferenceResult>::iterator it_multi_activity_buffer = multi_activity_buffer.begin(); it_multi_activity_buffer != multi_activity_buffer.end(); it_multi_activity_buffer++)
							{
								time_array_sum.push_back(it_multi_activity_buffer->second.count_);
							}
							total_time = accumulate(time_array_sum.begin(), time_array_sum.end(), 0);

							if (total_time >= alert_frame)
							{
								cout << "total_time: " << total_time << endl;
								int alert_result = 1;
								int longest_frame = 0;
								float alert_conf = 0.0;
								SortBuffer(multi_activity_buffer, &alert_result, &longest_frame, &alert_conf);

								cout << "-----> multi  activity: " << alert_result << endl;
								warning.warning_status_ = OutputAlert(alert_result, longest_frame, alert_conf);
								warning.warning_activity_ = alert_result;

							}

						}
					}
				}

			}

			state_previous = temp_inference_result.state_now_;

		}

		if (warning.warning_status_ == true)
		{
			cout << "warning status: " << warning.warning_status_ << "   warning activity: " << warning.warning_activity_ << endl;
		}

		frame_index++;

	}
	system("pause");
    return 0;
}