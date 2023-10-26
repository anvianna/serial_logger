#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <map>

struct Message {
    std::string id;
    std::string address;
};

int main(int argc, char* argv[]) {

    std::string file(argv[1]);
    std::cout << "file:" << file << std::endl;
    std::ifstream inputFile(file);
    if (!inputFile.is_open()) {
        std::cout << "Error opening input file." << std::endl;
        return 0;
    }

    std::map<std::string, Message> receivedMessages;
    std::map<std::string, Message> sentMessages;

    std::string line;
    std::regex dataRegex("\\Data P: (\\d+) - DEST: (\\w+) - SRC:(\\w+) - C0: (\\d+) - TTL: (\\d+)");
    std::regex logRegex("\\[LOG - ([A-Z])\\]-\\[(\\d+)\\]-\\[(\\w+)\\]-\\[(\\d+)\\]");


    Message AUX;
    while (std::getline(inputFile, line)) {
        std::smatch match;
        if(line.size() < 2) continue;
        if (std::regex_search(line, match, logRegex)) {
            std::string messageType = match[1];
            std::string messageId = match[2];
            std::string address = match[3];

            if (messageType == "R") {
                AUX.id = messageId;
                AUX.address = address;
                receivedMessages[messageId] = AUX;
            }
        } else if (std::regex_search(line, match, dataRegex)) {
            std::string messageId = match[1];
            std::string destination = match[2];
            std::string source = match[3];

            AUX.id = messageId;
            AUX.address = destination;
            sentMessages[messageId] = AUX;
        }
    }

    // Print received messages
    std::cout << "Received Messages:" << std::endl;
    for (const auto& entry : receivedMessages) {
        std::cout << "ID: " << entry.second.id << " Address: " << entry.second.address << std::endl;
    }

    // Print sent messages
    std::cout << "Sent Messages:" << std::endl;
    for (const auto& entry : sentMessages) {
        std::cout << "ID: " << entry.second.id << " Address: " << entry.second.address << std::endl;
    }

    std::cout << "Stats:" <<std::endl
        << "R:" << receivedMessages.size() << std::endl
        << "H:" << sentMessages.size() << std::endl;

    inputFile.close();
    return 0;
}
