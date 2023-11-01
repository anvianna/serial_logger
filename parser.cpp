#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <map>

struct Message {
    std::string dest_address;
    std::string n_hops;
};

typedef std::map<uint8_t,Message> id_message;

void print_map(std::map<std::string,id_message> &p_map)
{
    for(const auto& entry : p_map)
    {
        std::cout << "\nFrom:" << entry.first << std::endl;
        for(const auto& s_entry : entry.second)
        {
            std::cout << "ID:" << unsigned(s_entry.first) << "- Dest:" << s_entry.second.dest_address << " - Hops:" << s_entry.second.n_hops << std::endl;
        }
    }
}
void print_map(std::map<std::string,id_message> &p_map,std::ofstream &out_file)
{
    for(const auto& entry : p_map)
    {
        std::cout << "\nFrom:" << entry.first << std::endl;
        out_file << "\nFrom:" << entry.first << std::endl;
        for(const auto& s_entry : entry.second)
        {
            std::cout << "ID:" << unsigned(s_entry.first) << "- Dest:" << s_entry.second.dest_address << " - Hops:" << s_entry.second.n_hops << std::endl;
            out_file << "ID:" << unsigned(s_entry.first) << "- Dest:" << s_entry.second.dest_address << " - Hops:" << s_entry.second.n_hops << std::endl;
        }
    }
}

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

    std::map<std::string,id_message> Rx_Messages;
    std::map<std::string,id_message> Hop_Messages;
    std::map<std::string,id_message> Expired_Messages;

    std::string line;
    //std::regex dataRegex("\\Data P: (\\d+) - DEST: (\\w+) - SRC:(\\w+) - C0: (\\d+) - TTL: (\\d+)");
    //std::regex logRegex("\\[LOG - ([A-Z])\\]-\\ID[(\\d+)\\]-\\DEST[(\\w+)\\]-\\SRC[(\\d+)]-\\HOP[(\\+d)\\]");
    std::regex logRegex("\\[LOG - ([A-Z])\\]-ID\\[(\\d+)\\]-DEST\\[(\\w+)\\]-SRC\\[(\\w+)\\]-HOP\\[(\\d+)\\]");

    Message AUX;
    uint16_t received_msg = 0;
    uint16_t sent_msg = 0;
    uint16_t expired_msg = 0;

    while (std::getline(inputFile, line)) {
        std::smatch match;
        size_t pos;
        pos = line.find(':');
        if(pos == std::string::npos) continue;
        line = line.substr(pos);
        if(line.size() < 2 ) continue;
        //std::cout << line << std::endl;
        if (std::regex_search(line, match, logRegex)) {
            std::string messageType = match[1];
            std::string messageId = match[2];
            std::string dest_address = match[3];
            std::string src_address = match[4];
            std::string hops = match[5];

            AUX.dest_address = dest_address;
            AUX.n_hops = hops;

            uint8_t id = std::stoi(messageId);

            if (messageType == "R")
            {
                received_msg++;
                Rx_Messages[src_address][id] = AUX;
            }
            else if (messageType == "H")
            {
                sent_msg++;
                Hop_Messages[src_address][id] = AUX;
            }
            else if (messageType == "E")
            {
                expired_msg++;
                Expired_Messages[src_address][id] = AUX;
            }
        }/* else if (std::regex_search(line, match, dataRegex)) {
            std::string messageId = match[1];
            std::string destination = match[2];
            std::string source = match[3];

            AUX.id = messageId;
            AUX.address = destination;
            sentMessages[messageId] = AUX;
        }*/
    }

    // Print received messages
    /*
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
    // */

    std::string outfile = "Parsed_"+file;
    std::ofstream outputFile(outfile,std::ios::app);

    if(outputFile.is_open())
    {
        std::cout << "Received Messages:" << std::endl;
        outputFile << "Received Messages:" << std::endl;
        print_map(Rx_Messages,outputFile);
        outputFile << "Sent Messages:" << std::endl;
        print_map(Hop_Messages,outputFile);
        std::cout << "Expired Messages:" << std::endl;
        outputFile << "Expired Messages:" << std::endl;
        print_map(Expired_Messages,outputFile);

        outputFile << "Stats:" <<std::endl
        << "R:" << received_msg << std::endl
        << "H:" << sent_msg << std::endl
        << "E:" << expired_msg << std::endl;

        std::cout << "Writed in:" << outfile << std::endl;
    }
    else {
        std::cout << "Could not write in output file" << std::endl;
        //std::cout << "Received Messages:" << std::endl;
        //print_map(Rx_Messages);
        //std::cout << "Sent Messages:" << std::endl;
        //print_map(Hop_Messages);
        //std::cout << "Expired Messages:" << std::endl;
        //print_map(Expired_Messages);
    }

    std::cout << "Stats:" <<std::endl
        << "R:" << received_msg << std::endl
        << "H:" << sent_msg << std::endl
        << "E:" << expired_msg << std::endl;


    inputFile.close();
    return 0;
}
