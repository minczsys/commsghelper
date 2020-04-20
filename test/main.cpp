#include<iostream>
#include "commsghelper.h"

using alpaca::commsg::string_tokenizer;
using std::stringstream;
using std::string;

struct AIEDMInfo
{
    unsigned char mark;
    string flag;
    int index;
    int sender;
    int reciver;
    string data;
    int state;
    unsigned char crcmark;
    string crc;
    DEFINE_COM_SERIALIZATION(mark, flag, index, sender, reciver, data, state, crcmark,crc)
};


int main()
{
    //反序列化
    AIEDMInfo decodemsg;
    auto test = string_tokenizer("$TEST,002,123456789,123456789,IiIjIjIhIiIiIiMiIiIyIyIiIyMyIiIyIyIiIjMhIiMzISIzIjIjIiIiIiIiMyIyIjITIiMiIiIyIiIiIzMiIiMyIiIiIiMiNCEjIjMiIjMhIiMiMyIzIjIj,0*5A");
    decodemsg.decode_com_state(test);
    std::cout << "mark: " << decodemsg.mark << std::endl;
    std::cout << "flag: " << decodemsg.flag << std::endl;
    std::cout << "index: " << decodemsg.index << std::endl;
    std::cout << "sender: " << decodemsg.sender << std::endl;
    std::cout << "reciver: " << decodemsg.reciver << std::endl;
    std::cout << "data: " << decodemsg.data << std::endl;
    std::cout << "state: " << decodemsg.state << std::endl;
    std::cout << "crcmark: " << decodemsg.crcmark << std::endl;
    std::cout << "crc: " << decodemsg.crc << std::endl;
    //序列化
    AIEDMInfo encodemsg;
    encodemsg.mark = '$';
    encodemsg.flag = "TEST";
    encodemsg.index = 2;
    encodemsg.sender = 123456789;
    encodemsg.reciver = 123456789;
    encodemsg.data = "IiIjIjIhIiIiIiMiIiIyIyIiIyMyIiIyIyIiIjMhIiMzISIzIjIjIiIiIiIiMyIyIjITIiMiIiIyIiIiIzMiIiMyIiIiIiMiNCEjIjMiIjMhIiMiMyIzIjIj";
    encodemsg.state = 0;
    encodemsg.crcmark = '*';
    encodemsg.crc = "5A";
    stringstream x;
    encodemsg.encode_json(x);
    std::cout << x.str() << std::endl;
    return 0;
}
