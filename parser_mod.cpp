#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <map>
#include <list>
#include <time.h>



struct Tx_request
{
    std::string Dest;
    std::string marker;
    bool replied = false;
};

struct Message {
    std::string dest_address;
    std::string n_hops;
    std::string timestamp;
    std::string data = "";
    int tsim;
};

int time_to_tsim(std::string time)
{
    int i =0;
    i+= std::stoi(time.substr(0,2))*60*60*1000;
    i+= std::stoi(time.substr(3,2))*60*1000;
    i+= std::stoi(time.substr(6,2))*1000;
    i+= std::stoi(time.substr(9))/1000;
    return i;
}

typedef std::map<unsigned int,Message> id_message;

bool compare_string(const std::string &S1,const std::string &S2)
{
    for(int i= 0; i < S2.size(); i++)
    {
        if(S1.at(i) != S2.at(i)) return false;
    }
    return true;
}

bool search_map(id_message &id_map, std::string &data, unsigned int id)
{
    for(const auto& s_entry : id_map)
    {
        if(s_entry.first != id && s_entry.second.data == data) return false;
    }
    return true;
}

void print_map(std::map<std::string,id_message> &p_map)
{
    int count;
    for(const auto& entry : p_map)
    {
        std::cout << "\nFrom:" << entry.first << std::endl;
        count = 0;
        for(const auto& s_entry : entry.second)
        {
            count++;
            std::cout << "ID:" << unsigned(s_entry.first) << "- Dest:" << s_entry.second.dest_address << " - Hops:" << s_entry.second.n_hops << "  TS:" << s_entry.second.timestamp << std::endl;
        }
        std::cout << "Total:" << count << std::endl;
    }
}
void print_map(std::map<std::string,id_message> &p_map,std::ofstream &out_file)
{
    int count;
    for(const auto& entry : p_map)
    {
        //std::cout << "\nFrom:" << entry.first << std::endl;
        out_file << "\nFrom:" << entry.first << std::endl;
        count = 0;
        for(const auto& s_entry : entry.second)
        {
            count++;
            //std::cout << "ID:" << unsigned(s_entry.first) << "- Dest:" << s_entry.second.dest_address << " - Hops:" << s_entry.second.n_hops << "  TS:" << s_entry.second.timestamp << "|" << s_entry.second.tsim <<  std::endl;
            out_file << "ID:" << unsigned(s_entry.first) << "- Dest:" << s_entry.second.dest_address << " - Hops:" << s_entry.second.n_hops << "  TS:" << s_entry.second.timestamp << "|" << s_entry.second.tsim << std::endl;
        }
        out_file << "Total:" << count << std::endl;
    }
}

int main(int argc, char* argv[]) {

    std::string file(argv[1]);
    //std::string file = "C_DISTRIBUTED_TX_54DA.txt";
    std::cout << "file:" << file << std::endl;
    std::string file2 = file;
    std::ifstream inputFile(file);
    if (!inputFile.is_open()) {
        std::cout << "Error opening input file." << std::endl;
        return 0;
    }

    int n_hops[7] = {0,0,0,0,0,0,0};

    std::map<std::string, Message> receivedMessages;
    std::map<std::string, Message> sentMessages;

    std::map<std::string,id_message> Rx_Messages;
    std::map<std::string,id_message> Hop_Messages;
    std::map<std::string,id_message> Expired_Messages;

    std::map<std::string,std::list<Tx_request>> ReplyMsgMap;

    std::list<Tx_request> ReplyMsgList;

    std::string line;
    //std::regex dataRegex("\\Data P: (\\d+) - DEST: (\\w+) - SRC:(\\w+) - C0: (\\d+) - TTL: (\\d+)");
    //std::regex logRegex("\\[LOG - ([A-Z])\\]-\\ID[(\\d+)\\]-\\DEST[(\\w+)\\]-\\SRC[(\\d+)]-\\HOP[(\\+d)\\]");
    std::regex logRegex("\\[LOG - ([A-Z])\\]-ID\\[(\\d+)\\]-DEST\\[(\\w+)\\]-SRC\\[(\\w+)\\]-HOP\\[(\\d+)\\]");
    std::regex dataRegex("DATA:A\\|(\\d+)");
    std::regex markerRegex("Store to \\[(\\w+)\\]-\\[(\\d+)\\]\\[T\\|(\\d+)\\]");
                        // "Storing msg \\[(\\d+)\\]\\[T\\|(\\d+)\\] to \\[(\\d+)\\]"

    Message AUX;
    uint16_t received_msg = 0;
    uint16_t sent_msg = 0;
    uint16_t expired_msg = 0;

    uint16_t replied_msg = 0;

    std::string messageType;
    std::string messageId;
    std::string dest_address;
    std::string src_address ;
    std::string hops;
    unsigned int id;
    bool flag = false;
    long long c = 0;
    while (std::getline(inputFile, line)) {
        c++;
        std::smatch match;
        size_t pos;
        pos = line.find(':');
        if(pos == std::string::npos) continue;
        std::string ts = line.substr(13,15);
        line = line.substr(pos);
        if(line.size() < 2 ) continue;
        //std::cout << c <<"|" <<  line << std::endl;
        if (std::regex_search(line, match, logRegex)) {
            messageType = match[1];
            messageId = match[2];
            dest_address = match[3];
            src_address = match[4];
            hops = match[5];

            AUX.dest_address = dest_address;
            AUX.n_hops = hops;
            AUX.timestamp = ts;
            AUX.tsim = time_to_tsim(ts);

            id = std::stoi(messageId);

            n_hops[std::stoi(hops) - 1]++;

            if (messageType == "R")
            {
                received_msg++;
                Rx_Messages[src_address][id] = AUX;
                flag = true;
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
        }
        ///*
        else if(std::regex_search(line,match,markerRegex))
        {
            Tx_request request;
            request.marker = match[3];
            request.Dest = match[1];
            request.replied = false;
            ReplyMsgList.push_front(request);
            std::cout << "Mark Regex:["<< match[2] <<"]["<< match[1] <<"]["<< match[3] <<"]" << std::endl;
            /*
            std::list<Tx_request>::iterator list_pointer;
            auto iter = ReplyMsgMap.find(Dest);
            if(iter == ReplyMsgMap.end()){
                std::list<Tx_request> tx_list;
                tx_list.push_front(request);
                ReplyMsgMap.insert(ReplyMsgMap.begin(),std::make_pair(Dest,tx_list));
            }
            else{
                iter->second.push_front(request);
            }
            //*/
        }
        ///*
        else if(std::regex_search(line, match, dataRegex))
        {
            if(!flag) continue;
            else flag = false;
            std::string data = match[1];
            std::string marker = data;
            std::cout << "Data Regex:[" << marker << "]" << std::endl;
            //std::cout << "Put in [" << src_address << "]" << "[" << id <<"]" << std::endl;
            //search_map(Rx_Messages[src_address],data);

            Rx_Messages[src_address][id].data = data;
            for (auto& entry: ReplyMsgList)
            {
                if(compare_string(entry.marker,marker))
                {
                    if(!entry.replied)
                    {
                        entry.replied = true;
                        replied_msg++;
                    }
                    break;
                }
            }
        }//*/

        /* else if (std::regex_search(line, match, dataRegex)) {
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
    std::cout << "Here123\n";
    std::string outfile = "ParsedMod_";//+file;
    std::cout << "Here321 "<< file2 << std::endl;
    outfile += file2;
    std::cout << outfile << std::endl;
    std::ofstream outputFile(outfile,std::ios::app);
    std::cout << "Here2\n";

    if(outputFile.is_open())
    {
        std::cout << "Here3\n";
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
        << "E:" << expired_msg << std::endl
        << "Re" << replied_msg << std::endl;

        std::cout << "Hops: ";
        outputFile << "Hops: ";
        for(short i = 0; i < 7; i++)
        {

            std::cout << n_hops[i] << "|";
            outputFile << n_hops[i] << "|";
        }
        outputFile << std::endl;
        std::cout << std::endl;

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
        << "E:" << expired_msg << std::endl
        << "Re:" << replied_msg << std::endl;
        ;



    inputFile.close();
    return 0;
}
