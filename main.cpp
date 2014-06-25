/*
 * main.cpp
 *
 *  Created on: Jun 23, 2014
 *      Author: fguo
 *
 */
#include <algorithm>
#include <curl/curl.h>
#include <fstream>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iostream>
#include "libgen.h"
#include <mutex>
#include <string>
#include "sstream"
#include <stdio.h>
#include "Tinyxml/tinyxml.h"
#include "timing.hpp"
#include <vector>
using namespace std;


DEFINE_int32(batch_size,32,"batch size");

typedef struct {
	int thread_id;
	vector<string> * url_list;
	mutable mutex* url_list_mu;
} PhotoToMetadataPoolThreadData;

size_t write_to_string(void *ptr, size_t size, size_t count,
		string *writerData) {
	if (writerData == NULL) {
		return 0;
	}
	writerData->append((char *) ptr, size * count);
	return size * count;
}

char* GetNewFileName(const string& url) {

	char* file_name = basename((char*) url.c_str());
	//cout << file_name << endl;
	return file_name;
}



bool DownloadImage(const string& url) {

	CURL *curl;
	curl = curl_easy_init();
	string page_data;

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_POST, 0L);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(curl, CURLOPT_USERAGENT,
				"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US) AppleWebKit/534.7 (KHTML, like Gecko) Chrome/7.0.517.41 Safari/534.7");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &page_data);

		double filesize = 0;
		try {
			curl_easy_perform(curl);
			curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD,
					&filesize);
			//cout << "size:" <<  sizeof(page_data.c_str())/sizeof(char) << endl;
		} catch (int e) {
			curl_easy_cleanup(curl);
			return false;
		} catch (const std::exception& ex) {
			curl_easy_cleanup(curl);
			return false;
		}

		string outfilename = "/home/fguo/Downloads/testdownload/";
		outfilename += GetNewFileName(url);
		//stringbuf buffer;
		//buffer.sputn()
		ofstream ofs(outfilename);
		//cin>>page_data;
		//ofs<<page_data;
		ofs.write(page_data.c_str(), filesize);
		ofs.close();
	} else {
		curl_easy_cleanup(curl);
		return false;
	}

	curl_easy_cleanup(curl);
	return true;
}

void* PoolThread(void *threadarg) {
	PhotoToMetadataPoolThreadData * thread_data =
			(PhotoToMetadataPoolThreadData *) threadarg;
	//cout << "the thread size is " << size << endl;
	while (thread_data->url_list->size()) {
		string url = "";
		{
			lock_guard < mutex > lock(*(thread_data->url_list_mu));
			if (thread_data->url_list->size() == 0) {
				break;
			}
			url = thread_data->url_list->back();
			thread_data->url_list->pop_back();
		}
		DownloadImage(url);
	}
	pthread_exit((void*) threadarg);

}



bool GoogleplusCrawler(string userid, string access_token,
		vector<string>& url_list) {

	string url = "https://picasaweb.google.com/data/feed/api/user/" + userid
			+ "?kind=photo&max-results=9999999&access_token=" + access_token;

	printf("%s\n", url.c_str());
	CURL *curl;

	curl = curl_easy_init();
	string page_data;

	if (curl) {

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(curl, CURLOPT_USERAGENT,
				"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US) AppleWebKit/534.7 (KHTML, like Gecko) Chrome/7.0.517.41 Safari/534.7");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &page_data);

		curl_easy_perform(curl);
		TiXmlDocument doc;
		doc.Parse((const char*) page_data.c_str(), 0, TIXML_ENCODING_UTF8);
		TiXmlPrinter printer;
		TiXmlPrinter printer1;
		string url;
		TiXmlElement* entryelement =
				doc.FirstChildElement("feed")->FirstChildElement("entry");
		//entryelement->Accept(&printer);
		//fprintf(stdout, "the entry element is %s\n", printer.CStr());
		for (entryelement; entryelement;
				entryelement = entryelement->NextSiblingElement()) {

			TiXmlElement* mediaelement = entryelement->FirstChildElement(
					"media:group");
			if (mediaelement) {
				TiXmlElement* urlelement = mediaelement->FirstChildElement(
						"media:content");
				if (urlelement) {

					url = urlelement->Attribute("url");
					string filename=GetNewFileName(url);
					if(filename.find(".png")!=std::string::npos||filename.find(".jpg")!=std::string::npos||filename.find(".jpeg")!=std::string::npos){
						url_list.push_back(url);
					}

					//DownloadImage(url);

				} else {
					printf("errorwith media content\n");
				}
			} else {
				printf("error with media group\n");
			}
		}
	}
	curl_easy_cleanup(curl);

	return true;
}
int GetBatchSize(const int& array_size) {
	int batch_size;
	if (FLAGS_batch_size > array_size) {
		batch_size = array_size;
	} else {
		batch_size = FLAGS_batch_size;
	}
	return batch_size;
}


bool ProcessPhotos(vector<string>& url_list) {
	std::mutex url_list_mu;
	vector<PhotoToMetadataPoolThreadData> thread_data;
	int batch_size = GetBatchSize(url_list.size());
	cout << "batch_size is: " << batch_size << endl;
	thread_data.resize(batch_size);
	pthread_t threads[batch_size];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (int i = 0; i < batch_size; ++i) {

		// Add more parameters if you want
		thread_data[i].thread_id = i;
		thread_data[i].url_list = &url_list;
		thread_data[i].url_list_mu = &url_list_mu;

		int rc = pthread_create(&threads[i], &attr, PoolThread,
				(void *) &thread_data[i]);
		if (rc) {
			LOG(WARNING)
					<< "ERROR; thread return code from pthread_create() is "
					<< rc << "\n";
		}
		usleep(20000); // in us
	}

	pthread_attr_destroy(&attr);
	for (int i = 0; i < batch_size; ++i) {
		// Extract a face with surrounding areas.
		void *status;
		int rc = pthread_join(threads[i], &status);
		if (rc) {
			LOG(WARNING)
					<< "ERROR; thread return code from pthread_join() is " << rc
					<< "\n";
		}
	}
	return true;
}

bool Crawl(const string& uid, const string& access_token,vector<string>& url_list) {
	string foldername = "";
	if (!GoogleplusCrawler(uid, access_token, url_list)) {
		return false;
	}
	return ProcessPhotos(url_list);
}

int main(int argc, char **argv) {
	string uid = "116257145287338083118";
	string access_token =
			"ya29.LgC9kLAIPj59aB4AAAAmxaCrH1QYf2d48RLagx6M-zVGOPKaiCyGxg5-a4AwWw";
	Timing clock;
	vector<string> url_list;
	clock.print("begin!");
	Crawl(uid,access_token,url_list);
	//GoogleplusCrawler(userid.c_str(), access_token.c_str(), url_list);
	/*printf("%i\n", (int)url_list.size());
	for (std::vector<string>::iterator i = url_list.begin();
			i != url_list.end(); ++i) {
		cout << *i << endl;
	}*/
	clock.print("done!");
	//("done");
	return 0;
}

