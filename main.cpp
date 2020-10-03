#include <iostream>
#include <fstream>
#include <vector>
#include <cpprest/http_client.h>
#include "json/json.h"

using namespace std;
using namespace std;
using namespace web::http;
using namespace web::http::client;

vector<string> global_destination;
vector<string> global_uri;

uri_builder create_uri(string ip) {
    string token = "";
    ifstream token_file("/Users/KangDroid/ipfind_token.txt");
    getline(token_file, token);
    uri_builder uri;
    uri.set_scheme("https");
    uri.set_host("ipinfo.io");
    uri.set_path(ip);
    uri.set_query("token="+token);
    return uri;
}

bool check_exists(string tmp) {
    for (int i = 0; i < global_destination.size(); i++) {
        if (tmp == global_destination[i]) {
            return true;
        }
    }
    return false;
}

void save_vector() {
    string path_save = "/Users/KangDroid/Desktop/vector_output.txt";
    ofstream ofs(path_save);

    for (int i = 0; i < global_destination.size(); i++) {
        ofs << global_destination[i] << endl;
    }
}

int main(void) {
    Json::Value root;
    Json::Reader json_reader;
    http_request request_tpr (methods::GET);
    ofstream out_file("/Users/KangDroid/Desktop/json_parsed.txt");
    ifstream ifs("/Users/KangDroid/Desktop/test.json");
    if (json_reader.parse(ifs, root) == false) {
        cout << "Cannot parse json" << endl;
        return -1;
    } else {
        cout << "Successfully parsed json" << endl;
    }

    int root_size = root.size();
    cout << "Size of root: " << root_size << endl;

    for (int i = 0; i < root_size; i++) {
        string ip_version = root[i]["_source"]["layers"]["ip"]["ip.version"].asString();

        if (ip_version != "4") {
            continue;
        }

        string source_val = root[i]["_source"]["layers"]["ip"]["ip.src"].asString();
        string dest_val = root[i]["_source"]["layers"]["ip"]["ip.dst"].asString();

        //cout << source_val << " --> " << dest_val << endl; 
        if (!check_exists(dest_val)) {
            global_destination.push_back(dest_val);
        }
    }

    // Create URI Vector
    for (int i = 0; i < global_destination.size(); i++) {
        global_uri.push_back(create_uri(global_destination[i]).to_string());
    }

    for (int i = 0; i < global_uri.size(); i++) {
        http_client client(global_uri[i]);
        web::json::value root_value;
        try {
            client.request(request_tpr).then([&root_value] (http_response hr) {
                root_value = hr.extract_json().get();
            }).wait();
        } catch (const exception& expn) {
            cout << expn.what() << endl;
        }

        if (!root_value["ip"].is_null()) {
            out_file << "Information of IP: " << root_value["ip"].as_string() << endl;
        }
        if (!root_value["bogon"].is_null()) {
            if (root_value["bogon"].as_bool()) {
                out_file << "\tBogon[Local?]: " << "true" << endl;
            } else {
                out_file << "\tBogon[Local?]: " << "false" << endl;
            }
            
        }
        if (!root_value["hostname"].is_null()) {
            out_file << "\tHostname: " << root_value["hostname"].as_string() << endl;
        }
        if (!root_value["region"].is_null()) {
            out_file << "\tRegion: " << root_value["region"].as_string() << endl;
        }
        if (!root_value["country"].is_null()) {
            out_file << "\tCountry: " << root_value["country"].as_string() << endl;
        }

        out_file << "ASN Info: " << endl;
        if (!root_value["asn"].is_null()) {
            if (!root_value["asn"]["name"].is_null()) {
                out_file << "\tASN Name: " << root_value["asn"]["name"].as_string() << endl;
            }
            if (!root_value["asn"]["domain"].is_null()) {
                out_file << "\tASN Domain: " << root_value["asn"]["domain"].as_string() << endl;
            }
            if (!root_value["asn"]["type"].is_null()) {
                out_file << "\tASN Type: " << root_value["asn"]["type"].as_string() << endl;
            }
        }
        out_file << "Company: " << endl;
        if (!root_value["company"].is_null()) {
            if (!root_value["company"]["name"].is_null()) {
                out_file << "\tCompany Name: " << root_value["company"]["name"].as_string() << endl;
            }
            if (!root_value["company"]["domain"].is_null()) {
                out_file << "\tCompany Domain: " << root_value["company"]["domain"].as_string() << endl;
            }
            if (!root_value["company"]["type"].is_null()) {
                out_file << "\tCompany Type: " << root_value["company"]["type"].as_string() << endl;
            }
        }

        out_file << endl << endl;
    }
    // File Save
    save_vector();
    return 0;
}