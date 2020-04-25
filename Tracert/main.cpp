#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <thread>
#include <chrono>
#include "Client.h"

using namespace std;

bool hasSymbol(string input, string symbol);
vector<string> split(const string& str, const string& delim);
string replaceBrackets(const string str);
string parseResponseFromJson(const string json, const string property);
void output(const string number, const string ip);
void loader();

const string API_URL = "ip-api.com";
const string API_SUBURL = "/json/";
const string API_FIELDS = "?fields=country,org,as";
bool tracertFinished = false;

int main()
{
	setlocale(LC_ALL, "Russian");
	cout << "Введите IP или адрес сайта: ";

	string input = "";
	cin >> input;
	while (!hasSymbol(input, "."))
	{
		cout << "Please, write correct input!" << endl;
		cin >> input;
	}

	//здесь долгий участок, поэтому запускаю во втором потоке loader
	thread load(loader);
	string result = "tracert " + input + " > " + "output.txt";
	system(result.c_str());

	load.detach();
	tracertFinished = true;
	cout << "\r   \r";

	string resultFromFile;
	ifstream file("output.txt");

	vector<vector<string>> arrayResponse;

	if (file.is_open())
	{
		while (getline(file, resultFromFile))
		{
			arrayResponse.push_back(split(resultFromFile, " "));
		}
	}
	file.close();
	remove("output.txt");

	cout << "Готово! Откройте файл result.txt";
	freopen("result.txt", "wt", stdout);

	//4 - строка, в которой полезная инфа для нас
	int shift;
	for (int i = 4; i < arrayResponse.size(); i++)
	{
		shift = arrayResponse[i].size() == 8 ? 7 : 8;
		if (arrayResponse[i].size() != 0 && arrayResponse[i][1] != "*")
		{
			output(arrayResponse[i][0], arrayResponse[i][shift]);
		}
		else
		{
			if (arrayResponse[i].size() != 0)
				output(arrayResponse[i][0], arrayResponse[1][4]);
			break;
		}
	}

	cin.get();
}

vector<string> split(const string& str, const string& delim)
{
	vector<string> tokens;
	size_t prev = 0, pos = 0;

	do
	{
		pos = str.find(delim, prev);
		if (pos == string::npos)
			pos = str.length();
		string token = str.substr(prev, pos - prev);
		if (!token.empty())
			tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());

	return tokens;
}

bool hasSymbol(string input, string symbol)
{
	return input.find(symbol) != string::npos;
}

string replaceBrackets(const string str)
{
	string result = "";

	if (str[0] == '[' || str[str.length() - 1] == ']')
		for (int i = 1; i < str.length() - 1; i++)
			result += str[i];
	else
		return str;

	return result;
}

string parseResponseFromJson(const string json, const string property)
{
	stringstream jsonEncoded(json);
	boost::property_tree::ptree root;
	boost::property_tree::read_json(jsonEncoded, root);

	if (root.empty())
		return "-";

	return root.get<string>(property);
}

void output(const string number, const string ip)
{
	string ipWithoutBrackets = replaceBrackets(ip);
	string generatedUrl = API_SUBURL + ipWithoutBrackets + API_FIELDS;
	Client* client = new Client(API_URL, generatedUrl);
	string response = client->getResponse();

	cout << number << ") ";
	cout << "IP: " << ipWithoutBrackets << " (";
	cout << "Country: " << parseResponseFromJson(response, "country") << ", ";
	cout << "AS: " << parseResponseFromJson(response, "as") << ", ";
	cout << "Organization: " << parseResponseFromJson(response, "org") << ")";
	cout << endl;

	delete client;
}

void loader()
{
	while (!tracertFinished) {
		cout << ".";
		this_thread::sleep_for(chrono::milliseconds(500));
		if (tracertFinished) break;
		cout << ".";
		this_thread::sleep_for(chrono::milliseconds(500));
		if (tracertFinished) break;
		cout << ".";
		this_thread::sleep_for(chrono::milliseconds(500));
		if (tracertFinished) break;
		cout << "\r   \r";
	}
}


