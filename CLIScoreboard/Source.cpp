#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std;
#include <cpr/cpr.h>
#include <json/reader.h>
#include <json/value.h>

string code;

std::string get_current_dir() {
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir(buff, FILENAME_MAX);
    string current_working_dir(buff);
    return current_working_dir;
}

void closeSession(string prevCode) {
    cpr::Response r = cpr::Post(cpr::Url{ "https://scoreboardServer.b3nd3rssbm.repl.co" },
        cpr::Payload{ {"purpose", "closeSession"}, {"code",prevCode} });
}


Json::Value decode(string text)
{

    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();

    Json::Value root;
    string errors;
    bool parsingSuccessful = reader->parse(text.c_str(), text.c_str() + text.size(), &root, &errors);
    return root;
}

void fileText(string name, string val) {
    ofstream fileWrite(name);
    fileWrite << val;
    fileWrite.close();
}

bool copy_file(const char* From, const char* To, std::size_t MaxBufferSize = 1048576)
{
    std::ifstream is(From, std::ios_base::binary);
    std::ofstream os(To, std::ios_base::binary);

    std::pair<char*, std::ptrdiff_t> buffer;
    buffer = std::get_temporary_buffer<char>(MaxBufferSize);

    //Note that exception() == 0 in both file streams,
    //so you will not have a memory leak in case of fail.
    while (is.good() and os)
    {
        is.read(buffer.first, buffer.second);
        os.write(buffer.first, is.gcount());
    }

    std::return_temporary_buffer(buffer.first);

    if (os.fail()) return false;
    if (is.eof()) return true;
    return false;
}

Json::Value jsonGet(Json::Value json, string itemName) {
    Json::Value ret;
    try {
        ret = json.get(itemName, "ERR");
    }
    catch (Json::LogicError msg) {
    }
    return ret;
}

void updateFiles() {
    cpr::Response r = cpr::Post(cpr::Url{ "https://scoreboardServer.b3nd3rssbm.repl.co" },
        cpr::Payload{ {"purpose", "check"}, {"code", code} });
    Json::Value items = decode(r.text);
    Json::Value p1 = jsonGet(items,"p1");
    Json::Value p2 = jsonGet(items, "p2");
    fileText("ScoreFiles/round.txt", jsonGet(items, "round").asString());
    fileText("ScoreFiles/p1.txt", jsonGet(p1, "tag").asString());
    fileText("ScoreFiles/p2.txt", jsonGet(p2, "tag").asString());
    fileText("ScoreFiles/p1Score.txt", jsonGet(p1, "score").asString());
    fileText("ScoreFiles/p2Score.txt", jsonGet(p2, "score").asString());
    string dir1 = get_current_dir() + "/char-icons/" + jsonGet(p1, "character").asString() + "/" + jsonGet(p1, "costume").asString() + ".png";
    string dir2 = get_current_dir() + "/ScoreFiles/p1Char.png";
    string dir3 = get_current_dir() + "/char-icons/" + jsonGet(p2, "character").asString() + "/" + jsonGet(p2, "costume").asString() + ".png";
    string dir4 = get_current_dir() + "/ScoreFiles/p2Char.png";
    copy_file(dir1.c_str(), dir2.c_str());
    copy_file(dir3.c_str(), dir4.c_str());
}

void initReq() {
    cpr::Response r = cpr::Post(cpr::Url{ "https://scoreboardServer.b3nd3rssbm.repl.co" },
        cpr::Payload{ {"purpose", "newSession"} });
    Json::Value codeHold = decode(r.text);
    cout << "Go to https://melee.b3nd3rssbm.repl.co/ or https://ult.b3nd3rssbm.repl.co/, depending on the game. Put in the code listed below, and fill out the other fields. Click submit and within 30 seconds the files in the ScoreFiles folder will \nupdate!";
    cout << "\nCode: " << jsonGet(codeHold, "code").asString();
    code = jsonGet(codeHold, "code").asString();
    cout << "\nDo not close this window until you are done with the current scoreboard session";
    ofstream thisCode("lastCode.ini");
    thisCode << code;
    thisCode.close();
}

void start() {
    for (;;)
    {
        updateFiles();
        Sleep(30000);
    }
}

void closeLast() {
    ifstream lastCode("lastCode.ini");
    string myText;
    getline(lastCode, myText);
    closeSession(myText);
}

int main() {
    closeLast();
    initReq();
    start();
    if (__cplusplus == 201703L) std::cout << "C++17\n";
    else if (__cplusplus == 201402L) std::cout << "C++14\n";
    else if (__cplusplus == 201103L) std::cout << "C++11\n";
    else if (__cplusplus == 199711L) std::cout << "C++98\n";
    else std::cout << "pre-standard C++\n";
}
