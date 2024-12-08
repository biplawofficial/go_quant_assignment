#include <iostream>
#include <curl/curl.h>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>
#include <ctime>
using json = nlohmann::json;
using namespace std;

//Global variables

CURL *curl,*curl1, *curlsell, *curledit, *curlgorder, *curlcancle, *curlpos;
string access_token = "";
string refresh_token = "";
int alive_time=0;

void save_auth(const string &accesstoken, const string &refreshtoken)
{
    
    json jsondata;
    jsondata["access_token"] = accesstoken;
    jsondata["refresh_token"] = refreshtoken;
    string filename = "tokens.json";
    ofstream file(filename);
    if (!file)
    {
        cerr << "Error opening file!" << endl;
    }
    else
    {
        file << jsondata.dump(4);
        file.close();
        cout << "tokens saved to " << filename << "\n";
    }
}
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}
void makerequest(const string &url, const string &type, const string &headerfile)
{
    CURLcode response;
    string readBuffer;
    struct curl_slist *headers = NULL;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if (type == "post")
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, headerfile.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, headerfile.length());
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        // persistent http to avoid repeated tcp handshaking and creating new connection each time
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        // response handling
        response = curl_easy_perform(curl);
        if (response != CURLE_OK)
        {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(response) << endl;
        }
        else
        {
            json jsonresponse = json::parse(readBuffer);
            access_token = jsonresponse["result"]["access_token"];
            refresh_token = jsonresponse["result"]["refresh_token"];
            alive_time = jsonresponse["result"]["expires_in"];
            save_auth(access_token, refresh_token);
        }
        // //cleanup
        curl_slist_free_all(headers);
        
    }
}
int main()
{
    //authentication code
    string json_data = R"(
        {
            "jsonrpc": "2.0",
            "method": "public/auth",
            "id": 0,
            "params": {
                "grant_type": "client_credentials",
                "client_id": "fauEC0I9",
                "client_secret": "YPE8AhveohOM3TFtsVXSArk797hYOTQLo9RGf9QebvA"
            }
        }
    )";

    cout << "Authenticating...\n";
    string url = "https://test.deribit.com/api/v2/public/auth";
    curl = curl_easy_init();
    if (curl)
        makerequest(url, "post", json_data);
    else
    {
        cerr << "Failed to initialize cURL session!" << endl;
    }

    // code to refresh token if expired

    /*management calls*/
    short input;
    int id = 0;
    
    while (true)
    {
        cout << "select the corresponding number\n1. Buy \n2. Sell \n3. Modify Order\n4. Cancle Order\n5. Get Order Book\n6. Current Positions\n0. Exit\n";
        cin >> input;
        switch (input)
        {
        case 1:
        {
            curl1 = curl_easy_init();
            long amount = 0;
            struct curl_slist *headers = NULL;
            string url2 = "https://test.deribit.com/api/v2/private/buy";
            string authHeader = "Authorization: Bearer " + access_token;
            string instrument_name, readBuffer;
            CURLcode response;

            cout<<"Enter Instrument-Name\n";
            cin>>instrument_name;
            cout<<"Enter Amount\n";
            cin>>amount;

            json order_jsondata = {
            {"method", "private/buy"},
            {"params", {
                    {"instrument_name", instrument_name},
                    {"amount", amount},
                    {"type", "market"}
                }},
                {"jsonrpc", "2.0"},
                {"id", id++}
            };

            
            
            string json_string = order_jsondata.dump();

            //headers
            headers = curl_slist_append(headers, authHeader.c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            //calls
            curl_easy_setopt(curl1, CURLOPT_URL, url2.c_str());
            curl_easy_setopt(curl1, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl1, CURLOPT_POSTFIELDS, json_string.c_str());
            curl_easy_setopt(curl1, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl1, CURLOPT_WRITEDATA, &readBuffer);

            //request + err-handling
            response = curl_easy_perform(curl1);
            if (response != CURLE_OK)
            {
                cout<<"Cannot Buy at this time for more info check log!\n";
                // cerr << "curl_easy_perform() failed: " << curl_easy_strerror(response) << endl;
            }
            else
            {
                cout<<amount<<" Amount for instrument: "<<instrument_name<<" bought successfully!\n";
                // cout << "Response: " << readBuffer << endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl1);
            break;
        }
        case 2:
        {
            curlsell = curl_easy_init();
            long amount = 0;
            struct curl_slist *headers = NULL;
            string url2 = "https://test.deribit.com/api/v2/private/sell";
            string authHeader = "Authorization: Bearer " + access_token;
            string instrument_name, readBuffer;
            CURLcode response;


            cout<<"Enter Instrument-Name\n";
            cin>>instrument_name;
            cout<<"Enter Amount\n";
            cin>>amount;

            json jsondata = {
                {"method", "private/sell"},
                {"params", {
                    {"instrument_name", instrument_name},
                    {"amount", amount},
                    {"type", "market"},
                    {"otoco_config", json::array()}
                }},
                {"jsonrpc", "2.0"},
                {"id", id++}
            };

            string json_string = jsondata.dump();

            //headers
            headers = curl_slist_append(headers, authHeader.c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            //calls
            curl_easy_setopt(curlsell, CURLOPT_URL, url2.c_str());
            curl_easy_setopt(curlsell, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curlsell, CURLOPT_POSTFIELDS, json_string.c_str());
            curl_easy_setopt(curlsell, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curlsell, CURLOPT_WRITEDATA, &readBuffer);

            //request + err-handling
            response = curl_easy_perform(curlsell);
            if (response != CURLE_OK)
            {
                cout<<"Internal error !\n";
                // cerr << "curl_easy_perform() failed: " << curl_easy_strerror(response) << endl;
            }
            else
            {
                cout<<amount<<" amount for instrument: "<<instrument_name<<" sold successfully!\n";
                // cout << "Response: " << readBuffer << endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curlsell);
            break;
        }
 

        case 3:
        {
            curlcancle = curl_easy_init();
            long amount = 0,price=0;
            struct curl_slist *headers = NULL;
            string url2 = "https://test.deribit.com/api/v2/private/edit";
            string authHeader = "Authorization: Bearer " + access_token;
            string order_id, readBuffer;
            CURLcode response;


            cout<<"Enter order_id\n";
            cin>>order_id;
            cout<<"Enter amount (default 0)\n";
            cin>>amount;
            cout<<"Enter value (default 0)\n";
            cin>>price;

                json jsondata = {
                {"method", "private/edit"},
                {"jsonrpc", "2.0"},
                {"id", 44},
                {"params", {
                    {"order_id", order_id},
                    {"price", price},
                    {"amount", amount}
                }}
            };


            string json_string = jsondata.dump();

            //headers
            headers = curl_slist_append(headers, authHeader.c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            //calls
            curl_easy_setopt(curlcancle, CURLOPT_URL, url2.c_str());
            curl_easy_setopt(curlcancle, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curlcancle, CURLOPT_POSTFIELDS, json_string.c_str());
            curl_easy_setopt(curlcancle, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curlcancle, CURLOPT_WRITEDATA, &readBuffer);

            //request + err-handling
            response = curl_easy_perform(curlcancle);
            if (response != CURLE_OK)
            {
                cout<<"Internal error !\n";
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(response) << endl;
            }
            else
            {
                cout << "Response: " << readBuffer << endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curlcancle);
            break;
        }
        case 4:
        {
            curlcancle = curl_easy_init();
            long amount = 0;
            struct curl_slist *headers = NULL;
            string url2 = "https://test.deribit.com/api/v2/private/cancel";
            string authHeader = "Authorization: Bearer " + access_token;
            string order_id, readBuffer;
            CURLcode response;


            cout<<"Enter Instrument-Name\n";
            cin>>order_id;

            json jsondata = {
                {"method", "private/cancel"},
                {"jsonrpc", "2.0"},
                {"id", id++},
                {"params", {
                    {"order_id", order_id }
                }}
            };


            string json_string = jsondata.dump();

            //headers
            headers = curl_slist_append(headers, authHeader.c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            //calls
            curl_easy_setopt(curlcancle, CURLOPT_URL, url2.c_str());
            curl_easy_setopt(curlcancle, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curlcancle, CURLOPT_POSTFIELDS, json_string.c_str());
            curl_easy_setopt(curlcancle, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curlcancle, CURLOPT_WRITEDATA, &readBuffer);

            //request + err-handling
            response = curl_easy_perform(curlcancle);
            if (response != CURLE_OK)
            {
                cout<<"Internal error !\n";
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(response) << endl;
            }
            else
            {
                cout << "Response: " << readBuffer << endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curlcancle);
            break;
        }
        case 5:
        {
            curlgorder = curl_easy_init();
            long depth = 0;
            struct curl_slist *headers = NULL;
            string url2 = "https://test.deribit.com/api/v2/public/get_order_book";
            string authHeader = "Authorization: Bearer " + access_token;
            string instrument_name, readBuffer;
            CURLcode response;

            cout<<"Enter Instrument-Name\n";
            cin>>instrument_name;
            cout<<"Enter Depth\n";
            cin>>depth;

           json jsondata = {
            {"method", "public/get_order_book"},
            {"jsonrpc", "2.0"},
            {"id", id++},
            {"params", {
                {"instrument_name", instrument_name},
                {"depth", depth}
            }}
        };

            string json_string = jsondata.dump();

            //headers
            headers = curl_slist_append(headers, authHeader.c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            //calls
            curl_easy_setopt(curlgorder, CURLOPT_URL, url2.c_str());
            curl_easy_setopt(curlgorder, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curlgorder, CURLOPT_POSTFIELDS, json_string.c_str());
            curl_easy_setopt(curlgorder, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curlgorder, CURLOPT_WRITEDATA, &readBuffer);

            //request + err-handling
            response = curl_easy_perform(curlgorder);
            if (response != CURLE_OK)
            {
                cout<<"Internal error !\n";
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(response) << endl;
            }
            else
            {
                cout << "Response: " << readBuffer << endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curlgorder);
            break;
        }
        case 6:
        {
            curlpos = curl_easy_init();
            long depth = 0;
            struct curl_slist *headers = NULL;
            string url2 = "https://test.deribit.com/api/v2/private/get_positions";
            string authHeader = "Authorization: Bearer " + access_token;
            string currency, readBuffer;
            CURLcode response;

            cout<<"Enter Currency name (or select any)\n";
            cin>>currency;

           json jsondata = {
                {"method", "private/get_positions"},
                {"jsonrpc", "2.0"},
                {"id", id++},
                {"params", {
                    {"currency", currency}
                }}
            };


            string json_string = jsondata.dump();

            //headers
            headers = curl_slist_append(headers, authHeader.c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            //calls
            curl_easy_setopt(curlpos, CURLOPT_URL, url2.c_str());
            curl_easy_setopt(curlpos, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curlpos, CURLOPT_POSTFIELDS, json_string.c_str());
            curl_easy_setopt(curlpos, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curlpos, CURLOPT_WRITEDATA, &readBuffer);

            //request + err-handling
            response = curl_easy_perform(curlpos);
            if (response != CURLE_OK)
            {
                cout<<"Internal error !\n";
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(response) << endl;
            }
            else
            {
                cout << "Response: " << readBuffer << endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curlpos);
            break;
        }
        case 0:
        {
            curl_easy_cleanup(curl);
            cout<<"Cleaning all the stuffs...\nConnection closed\n";
            return 0;
            break;
        }
        }
    }
}
