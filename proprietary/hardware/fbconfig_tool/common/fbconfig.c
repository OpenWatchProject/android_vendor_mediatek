#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "fbconfig.h"

static int tmp_val[128];
static int fd_conf;
static int data_type;
static CONFIG_RECORD record_cmd;
static MIPI_TIMING timing;

static int file_parse(char *path);

static char _help[] =
        "\n"
        "USAGE\n"
        "                Under /vendor/bin or /system/bin and run fbconfig"
        "    For Example:    ./fbconfig [COMMAND] <PARAMETER...>\n"
        "\n"
        "COMMAND\n"
        "       get_dsi_id \n"
        "\n"
        "       set_dsi_id \n"
        "\n"
        "       lcm_get_id \n"
        "\n"
        "       driver_ic_config \n"
        "\n"
        "       mipi_set_clock \n"
        "\n"
        "       mipi_set_ssc \n"
        "\n"
        "       mipi_set_timing \n"
        "\n"
        "       mipi_set_non_cc \n"
        "\n"
        "       fb_layer_dump \n"
        "\n"
        "       get_dsi_continuous \n"
        "\n"
        "       get_dsi_clk \n"
        "\n"
        "       test_lcm_type \n"
        "\n"
        "       get_dsi_ssc \n"
        "\n"
        "       get_dsi_lane_num \n"
        "\n"
        "       get_dsi_timing \n"
        "\n"
        "       get_dsi_te_enable \n"
        "\n"
        "       te_set_enable \n"
        "\n"
        "       lcm_get_esd \n"
        "\n"
        "       driver_ic_reset \n"
        "\n"
        "       get_misc \n"
        "\n"
        "       Reserved \n"
        ;

static char mipi_help[] =
        "\n"
        "\nUsage Example: ./fbconfig mipi_set_timing HS_ZERO 23 INDEX\n"
        "TIMCON0_REG:"  "LPX" "  " "HS_PRPR" "  "  "HS_ZERO" "  "  "HS_TRAIL\n"
        "\n"
        "TIMCON1_REG:" "TA_GO" "  " "TA_SURE" "  " "TA_GET" "  " "DA_HS_EXIT\n"
        "\n"
        "TIMCON2_REG:" "CLK_ZERO" "  " "CLK_TRAIL" "  " "CONT_DET\n"
        "\n"
        "TIMCON3_REG:" "CLK_HS_PRPR" "  " "CLK_HS_POST" "  " "CLK_HS_EXIT\n"
        "\n"
        "VDO MODE :" "HPW" "  " "HFP" "  " "HBP" "  " "VPW" "  " "VFP" "  " "VBP"
        "\n"
        ;

/*---------------------Driver IC Config V2 START-----------------------*/
/* parse the line of commands and save them into cmd_buf[]. */
/* returns the number of commands other wise a negative value for error code. */
/* when parse a line of commands ends, returns 0. */
int parse_line2cmd(char *line, unsigned int *cmd_buf, int *ins_num)
{
        char *p = NULL;
        char *digit = NULL;
        char *rec = NULL;
        unsigned int idx = 0;
        static int new_cmd;

        if (line == NULL || cmd_buf == NULL)
                return -1;

        p = strchr(line, '{');
        if (p != NULL) {     /* a line start with '{' */
                if (strlen(line) > 3)
                        p = p + 1;          /* skip '{' */
                new_cmd = 1;
        } else {
                if (strlen(line) < 3)
                        return -2;
                p = line;
        }

        if (new_cmd == 0)
                return -3;

        cmd_buf[idx] = (unsigned int)strtoul(p, NULL, 16);
        if (cmd_buf[idx] == 0)
                return -2; /* the first cmd shall not be ZERO? */
        printf("====>the instruction is :0x%8x\n", cmd_buf[idx]);

        idx++;
        *ins_num = idx;
        rec = p;
        digit = strtok(p, ","); /* the command line is split by ',' */
        do {
                digit = strtok(NULL, ",");
                if (digit != NULL) {
                        cmd_buf[idx] = (unsigned int)strtoul(digit, NULL, 16);
                        rec = digit;
                        printf("====>the instruction is :0x%8x\n", cmd_buf[idx]);
                        idx++;
                }
        } while (digit != NULL);

        *ins_num = idx;

        /* a '}' may be in the last string,it means a instruction end */
        if (strchr(rec, '}') != NULL) {
                new_cmd = 0;
                return 0;
        }
        return idx;
}
/*---------------------Driver IC Config V2 END-----------------------*/

#define MAX_IC_CMD_LEN 2048
static int driver_ic_config(int fd, char *path)
{
        FILE *file = NULL;
        char *p = NULL;
        unsigned int *ins_buf = NULL;
        int ret = 0;
        int ins_num = 0;
        char tmp[MAX_IC_CMD_LEN] = {0};

        printf("driver ic config file is: %s\n", path);
        file = fopen(path, "r");
        if (file != NULL) {
                if (fgets(tmp, MAX_IC_CMD_LEN, file) != NULL) {
                        if (tmp[strlen(tmp) - 1] != '\n') {   /* only one line? */
                                fclose(file);
                                file = NULL;
                                return 0;
                        }
                        tmp[strlen(tmp) - 1] = '\0';
                        if (strstr(tmp, "driver_ic_config_file:v2") == 0) {   /*to support v1 file type*/
                                fclose(file);
                                file = NULL;
                                ret = file_parse(path);
                                return ret;
                        }
                }

                /* if we have "driver_ic_config_file:v2" in the first line of config file */
                ins_buf = (unsigned int *)record_cmd.ins_array;
                record_cmd.ins_num = 0;
                while (fgets(tmp, MAX_IC_CMD_LEN, file) != NULL) {
                        if (tmp[strlen(tmp) - 1] == '\n')
                                tmp[strlen(tmp)-1] = '\0';
                        p = strstr(tmp, "MS:");
                        if (p != NULL) {
                                int ms = 0;
                                sscanf(p + 3, "0x%x", &ms);
                                record_cmd.ins_num = 1;
                                record_cmd.type = RECORD_MS;
                                record_cmd.ins_array[0] = ms;
                                printf("msleep :%d\n", ms);
                                ret = ioctl(fd_conf, DRIVER_IC_CONFIG, &record_cmd);
                                record_cmd.ins_num = 0;
                                continue;
                        }
                        p = strstr(tmp, "PIN:");
                        if (p != NULL) {
                                int enable = 0;
                                sscanf(p + 4, "0x%x", &enable);
                                record_cmd.ins_num = 1;
                                record_cmd.type = RECORD_PIN_SET;
                                record_cmd.ins_array[0] = enable;
                                printf("pin set :%d\n", enable);
                                ret = ioctl(fd_conf, DRIVER_IC_CONFIG, &record_cmd);
                                record_cmd.ins_num = 0;
                                continue;
                        }
                        /* a command line */
                        ret = parse_line2cmd(tmp, ins_buf, &ins_num);
                        if (ret > 0) {
                                ins_buf = ins_buf + ins_num;
                                record_cmd.ins_num = record_cmd.ins_num + ins_num;
                        } else if (ret == 0) {      /* a instruction parsed end */
                                record_cmd.type = RECORD_CMD;
                                record_cmd.ins_num = record_cmd.ins_num + ins_num;
                                ret = ioctl(fd, DRIVER_IC_CONFIG, &record_cmd);
                                ins_buf = (unsigned int*)record_cmd.ins_array;
                                record_cmd.ins_num = 0;
                                ins_num = 0;
                        }
                }
                ret = ioctl(fd, DRIVER_IC_CONFIG_DONE, 0);
                fclose(file);
                file = NULL;
        } else {
                printf("open file %s failed\r\n", path);
                ret = -1;
        }
        return ret;
}

/*---------------------Driver IC Config START-----------------------*/

static int format_to_instrut(void)
{
        int base_data = 0;
        /*
          0x29==>0x2902
          0x39==>0x3902
          0x15==>0x1500
          0x05==>0x0500
        */
        switch (data_type) {
        case 0x39:
                base_data = 0x3902;
                break;
        case 0x29:
                base_data = 0x2902;
                break;
        case 0x15:
                base_data = 0x1500;
                break;
        case 0x05:
                base_data = 0x0500;
                break;
        default:
                printf("No such data type ,error!!");
                break;
        }

        if (base_data == 0x3902 || base_data == 0x2902) {
                int ins_num = 1 + ((tmp_val[1] + 1) / 4) + (((tmp_val[1] + 1) % 4) ? 1 : 0);
                int *ins_array = (int *)malloc(sizeof(int) * ins_num);
                if (ins_array == NULL) {
                    printf("no memmory,malloc error!\n");
                    return -1;
                }
                ins_array[0] = ((tmp_val[1] + 1) << 16) + base_data;    /* (1)the first one instruction */
                int ins_index = 1;
                int tmp_vi = 2; /* tmp_val_index */
                printf("ins_num is %d; tmp_val[1] is %d\n", ins_num, tmp_val[1]);
                if (ins_num > 1) {
                        ins_array[ins_index] = tmp_val[0] + (tmp_val[tmp_vi] << 8) +
                                               (tmp_val[tmp_vi + 1] << 16) + (tmp_val[tmp_vi + 2] << 24);
                        /* (2) the first-2nd instruction */
                        tmp_vi = 5;
                        for (ins_index = 2; ins_index < ((tmp_val[1] + 1) / 4 + 1); ins_index++) {
                                ins_array[ins_index] = tmp_val[tmp_vi] + (tmp_val[tmp_vi + 1] << 8) +
                                                       (tmp_val[tmp_vi + 2] << 16) + (tmp_val[tmp_vi + 3] << 24);
                                tmp_vi += 4; /* (3)the middle instruction; */
                        }
                        if (((tmp_val[1] + 1) % 4 != 0) && (tmp_val[1] > 3)) { /* (4) the last instruction; */
                                ins_array[ins_index] = tmp_val[tmp_vi];
                                if (tmp_vi <= tmp_val[1])
                                        ins_array[ins_index] += (tmp_val[tmp_vi + 1] << 8);
                                if (tmp_vi + 1 <= tmp_val[1])
                                        ins_array[ins_index] += (tmp_val[tmp_vi + 2] << 16);
                        }
                        printf("\n the ins_index is %d\n", ins_index);
                        int z = 0;
                        for (z = 0; z < ins_num; z++)
                                printf("====>the instruction is :0x%x\n", ins_array[z]);


                        /***now the cmd instructions are stored in ins_array[]****/
                        record_cmd.ins_num = ins_num;
                        record_cmd.type = RECORD_CMD;
                        memcpy(record_cmd.ins_array, ins_array, sizeof(int) * ins_num);
                        free(ins_array);
                        ins_array = NULL;
                } else {
                        printf("only one instruction to apply!!\n");
                        free(ins_array);
                        ins_array = NULL;
                        return 0;
                }
        }/* at least 2 instructions */
        else if (base_data == 0x1500) {
                int tmp_inst = 0;
                tmp_inst = base_data + (tmp_val[0] << 16) + (tmp_val[2] << 24);
                record_cmd.ins_num = 1;
                record_cmd.type = RECORD_CMD;
                printf("====>the instruction is :0x%x\n", tmp_inst);
                record_cmd.ins_array[0] = tmp_inst;
        } else if (base_data == 0x0500) {
                int tmp_inst = 0;
                tmp_inst = base_data + (tmp_val[0] << 16);
                record_cmd.ins_num = 1;
                record_cmd.type = RECORD_CMD;
                printf("====>the instruction is :0x%x\n", tmp_inst);
                record_cmd.ins_array[0] = tmp_inst;
        }
        return 0;
}

/* check if there's any upper case 'X' in string. */
/* return 0 for not found and -1 for 'X' present, which is not allowed. */
static int check_upper_case(char *tmp)
{
        int ret = 0;
        while (*tmp != '\0') {
                if (*tmp == 'X') {
                        ret = -1;
                        printf("\nnow tmp is %c\n", *tmp);
                        break;
                }
                tmp++;
        }
        return ret;
}

/* convert command string to integer value and use DRIVER_IC_CONFIG to set. */
static int convert_to_int(char *tmp)
{
        int ret = 0;
        printf("\ni want to know the record ^0^:\n%s", tmp);
        if (strncmp(tmp, "TYPE", 4) == 0) {
                /* it's cmd line, parse it !! */
                /*format : CMD:ADDR:NUM:{PAR,PAR,.....}*/
                int addr = 0;
                int num_par = 0;
                sscanf(tmp + 5, "0x%x", &data_type);
                memset(tmp_val, 0x00, sizeof(int) * 128);
                sscanf(tmp + 14, "0x%x", &addr);
                sscanf(tmp + 19, "0x%x", &num_par);
                tmp_val[0] = addr;
                tmp_val[1] = num_par;
                printf("\nAddr is 0x%x\nNum_par is 0x%x\n", addr, num_par);
                tmp = tmp + 23; /*here is ":{"*/

                if (strncmp(tmp, ":{", 2) == 0)/*check !!*/
                        printf("till now all is right ! and next is cmd value!\n");
                else
                        printf("something is wrong before cmd value!! check please\n");

                size_t size = strlen(tmp);
                printf("the size of tmp[] is %zu\n", size);

                int n = 2; /* tmp_val[0] and tmp_val[1] is already set. */
                tmp = tmp + 2;
                while ((strncmp(tmp, "}", 1) != 0)) {
                        sscanf(tmp, "0x%x", &tmp_val[n]);
                        printf("data value is 0x%x\n", tmp_val[n]);
                        n++;
                        tmp += 5;
                }
                if (format_to_instrut() != 0)  /* just operate on record_cmd */
                    return -1;
                ret = ioctl(fd_conf, DRIVER_IC_CONFIG, &record_cmd);

        } else if (strncmp(tmp, "MS", 2) == 0) {
                /* not cmd line; */
                int ms = 0;
                sscanf(tmp + 3, "0x%x", &ms);
                record_cmd.ins_num = 1;
                record_cmd.type = RECORD_MS;
                record_cmd.ins_array[0] = ms;
                printf("run here -->msleep :%d\n", ms);
                ret = ioctl(fd_conf, DRIVER_IC_CONFIG, &record_cmd);
        } else if (strncmp(tmp, "PIN", 3) == 0) {
                int enable = 0;
                sscanf(tmp + 4, "0x%x", &enable);
                record_cmd.ins_num = 1;
                record_cmd.type = RECORD_PIN_SET;
                record_cmd.ins_array[0] = enable;
                printf("run here -->pin set :%d\n", enable);
                ret = ioctl(fd_conf, DRIVER_IC_CONFIG, &record_cmd);
        }
        return ret;
}

/* parse driver_ic_config file to commands and set them. */
/* return 0 for success and otherwise failure. */
static int file_parse(char *path)
{
        FILE *filed = NULL;
        char tmp[MAX_IC_CMD_LEN] = {0};
        int ret = 0;

        printf("config file is: %s\n", path);
        filed = fopen(path, "r");
        if (filed != NULL) {
                while (fgets(tmp, MAX_IC_CMD_LEN, filed) != NULL) {
                        ret = check_upper_case(tmp);
                        if (ret != 0) {
                                printf("Error!!!there is a upper case 'X' in config file \nLine: %s\n", tmp);
                                break;
                        }

                        ret = convert_to_int(tmp); /*parsing the record to tmp_val[128];*/
                        if (ret != 0) {
                                printf("Error!!!failed to convert the command and set \nLine: %s\n", tmp);
                                break;
                        }
                }
                fclose(filed);
                filed = NULL;
                return ret;
        } else {
                printf("can not open file in:%s", path);
                return -1;
        }
}

/*---------------------Driver IC Config END-----------------------*/

/* convert type string to type value, and operate on timing.type */
static void check_mipi_type(char *type)
{
        timing.type = MAX;
        if (strncmp(type, "HS_PRPR", 7) == 0)
                timing.type = HS_PRPR;
        else if (strncmp(type, "HS_ZERO", 7) == 0)
                timing.type = HS_ZERO;
        else if (strncmp(type, "HS_TRAIL", 8) == 0)
                timing.type = HS_TRAIL;
        else if (strncmp(type, "TA_GO", 5) == 0)
                timing.type = TA_GO;
        else if (strncmp(type, "TA_SURE", 7) == 0)
                timing.type = TA_SURE;
        else if (strncmp(type, "TA_GET", 6) == 0)
                timing.type = TA_GET;
        else if (strncmp(type, "DA_HS_EXIT", 10) == 0)
                timing.type = DA_HS_EXIT;
        else if (strncmp(type, "CLK_ZERO", 8) == 0)
                timing.type = CLK_ZERO;
        else if (strncmp(type, "CLK_TRAIL", 9) == 0)
                timing.type = CLK_TRAIL;
        else if (strncmp(type, "CONT_DET", 8) == 0)
                timing.type = CONT_DET;
        else if (strncmp(type, "CLK_HS_PRPR", 11) == 0)
                timing.type = CLK_HS_PRPR;
        else if (strncmp(type, "CLK_HS_POST", 11) == 0)
                timing.type = CLK_HS_POST;
        else if (strncmp(type, "CLK_HS_EXIT", 11) == 0)
                timing.type = CLK_HS_EXIT;
        else if (strncmp(type, "HPW", 3) == 0)
                timing.type = HPW;
        else if (strncmp(type, "HFP", 3) == 0)
                timing.type = HFP;
        else if (strncmp(type, "HBP", 3) == 0)
                timing.type = HBP;
        else if (strncmp(type, "VPW", 3) == 0)
                timing.type = VPW;
        else if (strncmp(type, "VFP", 3) == 0)
                timing.type = VFP;
        else if (strncmp(type, "VBP", 3) == 0)
                timing.type = VBP;
        else if (strncmp(type, "LPX", 3) == 0)
                timing.type = LPX;
        else if (strncmp(type, "SSC_EN", 6) == 0)
                timing.type = SSC_EN;
        else
                printf("No such mipi timing control option!!\n");
}

static PM_LAYER_INFO get_layer_size(unsigned int layer_id)
{
        PM_LAYER_INFO tmp;
        tmp.layer_size = 0;
        tmp.index = layer_id;
        tmp.fmt = 0;
        printf("[LAYER_DUMP]layer_id is %d\n", layer_id);

        ioctl(fd_conf, FB_LAYER_GET_INFO, &tmp);
        return tmp;
}

static int fbconfig_layer_dump(unsigned int layer_id)
{
        char *base = NULL;
        char *offset = NULL;
        BMF_HEADER bmp_file;
        int fd_store = -1;
        char store_path[30] = {0};
        PM_LAYER_INFO layer_info;
        unsigned int i = 0;

        sprintf(store_path, "%s%d%s", "/data/layer", layer_id, "_dump.bmp");
        fd_store = open(store_path, O_WRONLY | O_CREAT, 0644);
        if (fd_store < 0) {
                printf("[LAYER_DUMP]create /data/layerX_dump.bmp fail !!\n");
                return -1;
        }
        memset(&layer_info, 0, sizeof(PM_LAYER_INFO));
        memset(&bmp_file, 0, sizeof(bmp_file));

        layer_info = get_layer_size(layer_id);
        printf("[LAYER_DUMP]layer%d size is %dbyte\n", layer_id, layer_info.layer_size);

        if (layer_info.layer_size > 0) {
                bmp_file.type = 0x4D42; /* 'BM' */
                bmp_file.fsize = layer_info.layer_size + 54;
                bmp_file.res1 = 0;
                bmp_file.res2 = 0;
                bmp_file.offset = 54; /* 40+14 bytes */
                /* below 40 bytes are for BMP INFO HEADER */
                bmp_file.this_struct_size = 0x28;
                bmp_file.width = (layer_info.layer_size)/(layer_info.height)/(layer_info.fmt/8);
                bmp_file.height = layer_info.height;
                bmp_file.planes = 0x01;
                bmp_file.bpp = layer_info.fmt; /* 32 */
                bmp_file.compression = 0x0;
                bmp_file.raw_size = layer_info.layer_size;
                bmp_file.x_per_meter = 0x0ec4;
                bmp_file.y_per_meter = 0x0ec4;
                bmp_file.color_used = 0x0;
                bmp_file.color_important = 0x0;

                printf("[LAYER_DUMP]size of bmp_file is %zu\n", sizeof(bmp_file));
                printf("[LAYER_DUMP]file size  is 0x%x\n", (layer_info.layer_size+54));
                printf("[LAYER_DUMP]raw size  is 0x%x\n", layer_info.layer_size);
                printf("[LAYER_DUMP]height  is %d\n", layer_info.height);
                printf("[LAYER_DUMP]bpp  is %d\n", layer_info.fmt);

                base = (char *)malloc(layer_info.layer_size);
                if (base == NULL) {
                        printf("[LAYER_DUMP]malloc for layer dump fail !!!\n");
                        close(fd_store);
                        fd_store = -1;
                        return -1;
                }
                printf("[LAYER_DUMP]malloc :0x%lx \n", (unsigned long)base);
                if (ioctl(fd_conf, FB_LAYER_DUMP, base) != 0) {
                        printf("[LAYER_DUMP]ioctl memcpy fail !!!\n");
                        close(fd_store);
                        fd_store = -1;
                        free(base);
                        base = NULL;
                        return -2;
                }
                write(fd_store, &bmp_file, sizeof(bmp_file)); /* write BMF header to bmp file; */
                offset = base + layer_info.layer_size; /* reposition pointer; */
                for (i = 0; i < layer_info.height; i++) { /* write raw data to bmp file; */
                        offset = offset - bmp_file.width * (layer_info.fmt / 8);
                        write(fd_store, offset, bmp_file.width * (layer_info.fmt / 8));
                }
                free(base);
                base = NULL;
                offset = NULL;
                close(fd_store);
                fd_store = -1;
                return 0;
        } else {
                close(fd_store);
                fd_store = -1;
                return -2;
        }
}
#define FBCONFIG_FILE_PATH      "/sys/kernel/debug/fbconfig"
int main(int argc, char **argv)
{
        int ret = 0;
        int i = 0;

        if (argc < 2) {
                printf("%s", _help);
                return -1;
        }

        for (i = 0; i < argc; i++)
                printf("%s ", argv[i]);
        printf("\r\n");

        fd_conf = open(FBCONFIG_FILE_PATH, O_RDWR);
        if (fd_conf <= 0) {
                printf(" \n***open %s Fail!!*** \n", FBCONFIG_FILE_PATH);
                return -1;
        }
        if (strncmp(argv[1], "get_dsi_id", 10) == 0) {
                unsigned int dsi_id = 0;
                ret = ioctl(fd_conf, GET_DSI_ID, &dsi_id);
                printf("get_dsi_id:%d", dsi_id);
        } else if (strncmp(argv[1], "set_dsi_id", 10) == 0) {
                if (argc != 3) {
                        printf("\nUsage: <./fbconfig set_dsi_id ID>\n");
                        ret = 0;
                        goto end;
                } else {
                        unsigned int dsi_id = (unsigned int)atoi(argv[2]);
                        ret = ioctl(fd_conf, SET_DSI_ID, &dsi_id);
                        printf("set_dsi_id:%d", dsi_id);
                }
        } else if (strncmp(argv[1], "lcm_get_id", 10) == 0) {
                unsigned int id_num = 0;
                ret = ioctl(fd_conf, LCM_GET_ID, &id_num);
                printf("lcm_get_id :%d", id_num);
                if (id_num == 0)
                        printf("\n====please make sure you have implemented get_lcm_id() in lcm driver==");
        } else if (strncmp(argv[1], "driver_ic_config", 16) == 0) {
                /*lcm driver IC config, this will parse config file and process in lcm driver.*/
                if (argc != 3) {
                        printf("\nUsage: <./fbconfig driver_ic_config [file path]>\n");
                        ret = 0;
                        goto end;
                } else {
                        char path[4096] = {0};
                        int x = 0;
                        if (strlen(argv[2]) < 4089) {
                                sprintf(path, "/data/%s", argv[2]);
                                memset(&record_cmd, 0x00, sizeof(CONFIG_RECORD));
                                ret = driver_ic_config(fd_conf, path);
                        } else {
                                printf("\nThe name of config file is too long !!\n");
                                ret = -1;
                        }
                }
        } else if (strncmp(argv[1], "mipi_set_clock", 14) == 0) {
                if (argc != 3) {
                        printf("\nUsage: <./fbconfig mipi_set_clock CLOCK>\n");
                        ret = 0;
                        goto end;
                } else {
                        unsigned int clock = (unsigned int)atoi(argv[2]);
                        ret = ioctl(fd_conf, MIPI_SET_CLK, &clock);
                        printf("mipi_set_clock :%d", clock);
                }
        } else if (strncmp(argv[1], "mipi_set_ssc", 12) == 0) {
                if (argc != 3) {
                        printf("\nUsage: <./fbconfig mipi_set_ssc SSC_RANGE>\n");
                        ret = 0;
                        goto end;
                } else {
                        unsigned int ssc = (unsigned int)atoi(argv[2]);
                        /*maybe need deal with INDEX(dsi0,dsi1....)later,but not now*/
                        ret = ioctl(fd_conf, MIPI_SET_SSC, &ssc);
                        printf("mipi_set_ssc=>dsi:%d", ssc);
                }
        } else if (strncmp(argv[1], "mipi_set_timing", 15) == 0) {
                if (argc != 4) {
                        printf("%s\n", mipi_help);
                        ret = 0;
                        goto end;
                }
                timing.value = (unsigned int)atoi(argv[3]);
                check_mipi_type(argv[2]);
                printf("mipi_set_timing :type is %d;value is %d\n", timing.type, timing.value);
                ret = ioctl(fd_conf, MIPI_SET_TIMING, &timing);
                if (ret != 0) {
                        printf("==Error !! Do you have ever put your phone in suspend mode ?==\n");
                        printf("==Please make sure your phone NOT in suspend mode!!");
                }
        } else if (strncmp(argv[1], "mipi_set_non_cc", 15) == 0) {
                if (argc != 3) {
                        printf("\nUsage: <./fbconfig mipi_set_non_cc [value]\n");
                        ret = 0;
                        goto end;
                } else {
                        unsigned int cc_en = (unsigned int)atoi(argv[2]);
                        ret = ioctl(fd_conf, MIPI_SET_CC, &cc_en);
                        printf("\nmipi_set_non_cc =>dsi:%d", cc_en);
                }
        } else if (strncmp(argv[1], "fb_layer_dump", 13) == 0) {
                PM_LAYER_EN layers;
                int get_info_ok = 0;
                struct misc_property misc;

                if (ioctl(fd_conf, FB_LAYER_GET_EN, &layers) == 0) {
                        if (ioctl(fd_conf, FB_GET_MISC, &misc) == 0) {
                                printf("The current layer enable/disable info is :\n");
                                unsigned int i = 0;
                                for (i = 0; i < misc.overall_layer_num; ++i)
                                        printf("layer_%d:%s\n", i, (layers.layer_en[i] == 1) ? "Enable" : "Disable");
                                get_info_ok = 1;
                        } else {
                                printf("get layer info failed.\n");
                        }
                }

                if (get_info_ok && argc >= 3) {
                        unsigned int layer_id = (unsigned int)atoi(argv[2]);
                        if (layer_id >= misc.overall_layer_num) {
                                printf("Currently we only have totally %d layer to be dumped !!", misc.overall_layer_num);
                        } else {
                                if (layers.layer_en[layer_id] != 0) {
                                        int ret = fbconfig_layer_dump(layer_id);
                                        if (ret ==0)
                                                printf("Layer dump Correctly!!");
                                        else
                                                printf("Layer dump Fail!!");
                                } else {
                                        printf("Layer %d currently is not available.\n", layer_id);
                                }
                        }
                } else {
                        printf("\nUsage: <./fbconfig fb_layer_dump LAYER_ID> to dump LAYER_ID\n");
                }
        } else if (strncmp(argv[1], "get_dsi_continuous", 18) == 0) {
                unsigned int continue_clk = 0;
                ret = ioctl(fd_conf, LCM_GET_DSI_CONTINU, &continue_clk);
                printf("get_dsi_continuous=>dsi:%d\n", continue_clk);
        } else if (strncmp(argv[1], "get_dsi_clk", 11) == 0) {
                unsigned int dsi_clk = 0;
                ret = ioctl(fd_conf, LCM_GET_DSI_CLK, &dsi_clk);
                printf("get_dsi_clk=>dsi:%d\n", dsi_clk);
        } else if (strncmp(argv[1], "test_lcm_type", 13) == 0) {
                LCM_TYPE_FB lcm_fb;
                lcm_fb.clock = 0;
                lcm_fb.lcm_type = 0;
                ret = ioctl(fd_conf, LCM_TEST_DSI_CLK, &lcm_fb);
                printf("get_dsi_type ==>clk:%d \n", lcm_fb.clock);
                /* Note: the mode value
                 * CMD_MODE = 0,
                 * SYNC_PULSE_VDO_MODE = 1,
                 * SYNC_EVENT_VDO_MODE = 2,
                 * BURST_VDO_MODE = 3
                 */
                switch (lcm_fb.lcm_type) {
                case 0:
                        printf("get_dsi_type ==> CMD_MODE\n");
                        break;
                case 1:
                        printf("get_dsi_type ==> SYNC_PULSE_VDO_MODE \n");
                        break;
                case 2:
                        printf("get_dsi_type ==> SYNC_EVENT_VDO_MODE\n");
                        break;
                case 3:
                        printf("get_dsi_type ==> BURST_VDO_MODE\n");
                        break;
                default:
                        printf("get_dsi_type ==> Error: no such type!!\n");
                        break;
                }
        } else if (strncmp(argv[1], "get_dsi_ssc", 11) == 0) {
                unsigned int ssc = 0;
                ret = ioctl(fd_conf, LCM_GET_DSI_SSC, &ssc);
                printf("get_dsi_ssc=> ssc:%d\n", ssc);
        } else if (strncmp(argv[1], "get_dsi_lane_num", 16) == 0) {
                unsigned int dsi_lane_num = 0;
                ret = ioctl(fd_conf, LCM_GET_DSI_LANE_NUM, &dsi_lane_num);
                printf("get_dsi_lane_num=>dsi:%d\n", dsi_lane_num);
        } else if (strncmp(argv[1], "get_dsi_timing", 14) == 0) {
                if (argc != 3) {
                        printf("\nUsage: <./fbconfig get_dsi_timing VBP>\n");
                        ret = 0;
                        goto end;
                } else {
                        check_mipi_type(argv[2]);
                        timing.value = 0;
                        ret = ioctl(fd_conf, LCM_GET_DSI_TIMING, &timing);
                        printf("get_dsi_timing==>%s:%d\n", argv[2], timing.value);
                }
        } else if (strncmp(argv[1], "get_dsi_te_enable", 17) == 0) {
                unsigned int dsi_te_enable = 0;
                ret = ioctl(fd_conf, LCM_GET_DSI_TE, &dsi_te_enable);
                printf("get_dsi_te_enable:%d\n", dsi_te_enable);
        } else if (strncmp(argv[1], "te_set_enable", 13) == 0) {
                if (argc != 3) {
                        printf("\nUsage: <./fbconfig te_set_enable ENABLE>\n");
                        ret = 0;
                        goto end;
                } else {
                        unsigned int dsi_te_enable = (unsigned int)atoi(argv[2]);
                        ret = ioctl(fd_conf, TE_SET_ENABLE, &dsi_te_enable);
                        printf("te_set_enable=>dsi:%d", dsi_te_enable);
                }
        } else if (strncmp(argv[1], "lcm_get_esd", 11) == 0) {
                unsigned int addr = 0;
                unsigned int para_num = 0;
                unsigned int type = 0;
                ESD_PARA esd_para;
                if (argc != 5) {
                        printf("./fbconfig lcm_get_esd [address] [type] [paramter num]\r\n");
                        ret = 0;
                        goto end;
                }

                printf("%s %s %s %s %s\r\n", argv[0], argv[1], argv[2], argv[3], argv[4]);
                addr = (unsigned int)strtoul(argv[2], NULL, 16);
                type = (unsigned int)strtoul(argv[3], NULL, 16);
                para_num = (unsigned int)strtoul(argv[4], NULL, 16);
                if (para_num > 4) {
                        printf("the para_num must less than 4!!\n");
                        ret = 0;
                        goto end;
                }
                printf("lcm_get_esd:addr=0x%x type=%d para_num=%d\n", addr, type, para_num);
                printf("lcm_get_esd:type==0 means:DCS Read;  type==1 means GERNERIC READ\n");

                esd_para.addr = addr;
                esd_para.type = type;
                esd_para.para_num = para_num;
                esd_para.esd_ret_buffer = malloc(sizeof(char) * (para_num + 6));
                if (esd_para.esd_ret_buffer == NULL) {
                        ret = -1;
                        goto end;
                }

                memset(esd_para.esd_ret_buffer, 0, (para_num + 6));
                if (ioctl(fd_conf, LCM_GET_ESD, &esd_para) == 0) {
                        unsigned int i = 0;
                        for (i = 0; i < (para_num + 6); i++) {
                                printf("\nLCM_GET_ESD:esd_get[%d]==>0x%x\n", i, esd_para.esd_ret_buffer[i]);
                        }
                        ret = 0;
                } else {
                        printf("Something WRONG in LCM_GET_ESD\n");
                        ret = -1;
                }
                free(esd_para.esd_ret_buffer);
                esd_para.esd_ret_buffer = NULL;
        } else if (strncmp(argv[1], "driver_ic_reset", 15) == 0) {
                if (argc != 2) {
                        printf("\nUsage: <./fbconfig driver_ic_reset > \n");
                        ret = 0;
                        goto end;
                } else {
                        ret = ioctl(fd_conf, DRIVER_IC_RESET, NULL);
                        printf("\nIn order to Reset Driver IC config to lcm_init setting\n");
                }
        } else if (strncmp(argv[1], "get_misc", 8) == 0) {
                struct misc_property misc;
                ret = ioctl(fd_conf, FB_GET_MISC, &misc);
                printf("get_misc: 0x%08x\n", *(unsigned int *)&misc);
                printf("reserve : %08x\n", misc.reserved);
                printf("dual_port: %d\n", misc.dual_port);
        } else {
                ret = 0;
                printf("parameter is not correct !!%s", _help);
        }
end:
        close(fd_conf);
        fd_conf = -1;
        printf(" \n***finish for this query or setting ret=%d*** \n", ret);
        return ret;
}

