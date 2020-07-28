
#define ERRORINFO(msg) "#$ERROR^&_" #msg
class RPCClient;


#define OP_NAME_LEN 30
#define FULL_PRJ_NAME_LEN 60
#define RSC_FIELD_SIZE 256
typedef union {
        struct {
                unsigned int magic_number;
                unsigned int version;
                unsigned int dtbo_index;
                unsigned char full_project_name[FULL_PRJ_NAME_LEN];
                unsigned char op_name[OP_NAME_LEN];
        } rsc_info;
        unsigned char reserve[RSC_FIELD_SIZE];
} rsc_field_t;


class RscAssist
{
public:
    RscAssist();
    virtual ~RscAssist();
    static void switchProject(RPCClient* msgSender);

private:

    static int getPartPath(char* path, int size);
    static void fillData(RPCClient* msgSender, rsc_field_t* data);
    static int writePart(RPCClient* msgSender, rsc_field_t* data);
    static int extractInt(RPCClient* msgSender);
    static void extractStringToUnsighed(RPCClient* msgSender, unsigned char* outstr);
    static void extractString(RPCClient* msgSender, char* outstr);
    static int writeRawData(char* path, rsc_field_t* data, long long offset);
    static void clearItems();


};

