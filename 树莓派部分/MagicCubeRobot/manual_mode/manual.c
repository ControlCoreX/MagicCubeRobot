//! 这个手动模式里, 就只发送命令, 但是命令是带有参数的, 而且参数就固定是3个, 所以命令包(包括命令的数据包)的长度就是固定16个bytes.
//! 既然发送命令的时候都是, 标识符c, 索引len, 然后带3个uint32_t类型的参数, 但是发送的时候是字节流, 所以我要定义一个将这个数据包转化为字节流的函数. 
//! 然后这个manual.c中要发送命令, 就直接调用这个函数就可以了.
//! 把这个函数定义在tucom/tucom.c中.

//! 因为设计为固定都是3个参数, 所以当所需要的参数少于3个时, 靠前放. 即: 如果只需要一个参数, 那就把它放到第一个位置; 如果需要两个参数, 那就放前两个位置.
//! 关于函数参数的设计问题:
//! 因为是固定3个参数, 但有的函数不需要那么多参数, 怎么办? 这里采用的原则是:
//! 最后一环再决定否使用参数, 而中间环节把3个参数都带上. 这样可以保证统一性!
//! 即: 1. 命令行解析时3个参数都解析出来,
//!     2. switch-case时, 把这3个参数也都作为实参, (好处: 函数调用形式统一)
//!     3. 发送给单片机时, 把这3个参数都发送出去, (好处: 发送时函数调用的形式统一)
//!     4. 单片机接收到数据包, 是字节流, 要解析成为3个uint32_t类型, 此时也全部把3个参数解析出来, (好处: 定义解析参数的函数方便, 有意义)
//!     5. 解析完成, 使用函数指针去调用指定的函数实现指定功能时, 使用函数指针调用的时候, 也把3个参数都带上, (好处: 函数指针的定义形式固定, 即定义的实现功能的函数形式统一)
//!     6. 现在到了最后一环节: 在功能函数内部了, 3个参数也都传进来了, 此时, 是否使用以及使用哪些参数, 由用户自行实现决定.
//! 既然这样, 那过程中肯定会需要传一些实际用不到的参数, 在代码中对于这样的参数我用invalid做了标记. 其值为1.
//! 仅在无效参数的来源处, 即读取命令行的地方使用标记了, 是一个宏 #define INVALID_CHAR '1' , 其它处无需使用标记, 即把它这个参数当做是有效的看待, 与其他参数不必区分.

#include <wiringPi.h>
#include <wiringSerial.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "colorful.h"
#include "tucomm.h"
#include "manual.h"

//! 命令的个数.
#define NUM_OF_CMD  10U

//! 用1表示无效的参数.
#define INVALID_CHAR '1'

int fd = 0;//!< 串口文件描述符.

void get_cmd_and_args(char* str, char* cmd, char* arg1, char* arg2, char* arg3);
uint32_t str2uint32(char* str);



//! *
void quit(uint32_t param1, uint32_t param2, uint32_t param3)
{
    close(fd);//!< 关闭文件.
    exit(0);
}

void lscmd(uint32_t param1, uint32_t param2, uint32_t param3)
{
    extern char* cmd_table[];

    printf("\n" COLOR_BOLD_WHITE);
    for(int i = 0; i < NUM_OF_CMD; i++)
    {
        printf(" %.2d. %s\n", i, cmd_table[i]);
    }
    printf("\n" COLOR_OFF);
}

void help(uint32_t param1, uint32_t param2, uint32_t param3)
{
    //! 打印一些帮助信息.
    printf("\n" COLOR_BOLD_WHITE);
    printf("  step (index:0~10, dir:0/1, steps)\n\n");

    printf("  servo (cmp:50~250)\n\n");

    printf("  mcrota (dir:0/1, angle:90/180)\n\n");

    printf("  mcio, guideway1: 1 -> in\n");
    printf("  d2ud, guideway2: 1 -> up\n");
    printf("  hooc, guideway3: 1 -> close\n");

    printf("\n" COLOR_OFF);
}


void reset(uint32_t param1, uint32_t param2, uint32_t param3)
{
    tucomm_write_cmd(fd, 0U, param1, param2, param3);
}

void step(uint32_t index, uint32_t dir, uint32_t steps)
{
    if(dir != 0U)//!< 如果dir不为0, 它为其他值都认为它是1. 提高程序鲁棒性, 不会因为用户输入0/1以为的值而崩溃.
        dir = 1U;

    if(index > 10U)
    {
        printf(COLOR_BOLD_RED "index is too big, try again!\n" COLOR_OFF);
        return ;
    }

    tucomm_write_cmd(fd, 1U, index, dir, steps);
}

void servo(uint32_t cmp, uint32_t param2, uint32_t param3)
{
    if(cmp < 40U || cmp > 260)//!< 处理非法输入.
    {
        printf(COLOR_BOLD_RED "argument error, try again!\n" COLOR_OFF);
        return ;
    }

    tucomm_write_cmd(fd, 2U, cmp, param2, param3);
}

void mcrota(uint32_t dir, uint32_t angle, uint32_t param3)
{
    if(dir != 0U)
        dir = 1U;

    if(angle != 90U || angle != 180)//!< 处理非法输入.
    {
        printf(COLOR_BOLD_RED "argument error, try again!\n" COLOR_OFF);
        return ;
    }

    tucomm_write_cmd(fd, 3U, dir, angle, param3);
}

void mcio(uint32_t io, uint32_t param2, uint32_t param3)
{
    if(io != 0U)
        io = 1U;

    tucomm_write_cmd(fd, 4U, io, param2, param3);
}

void d2ud(uint32_t ud, uint32_t param2, uint32_t param3)
{
    if(ud != 0U)
        ud = 1U;
    
    tucomm_write_cmd(fd, 5U, ud, param2, param3);
}

void hooc(uint32_t oc, uint32_t param2, uint32_t param3)
{
    if(oc != 0U)
        oc = 1U;
    
    tucomm_write_cmd(fd, 6U, oc, param2, param3);
}

//! 函数指针表.
cmd_func_ptr cmd_funcs[NUM_OF_CMD] = 
{
    quit,
    lscmd,
    help,

    reset,
    step,
    servo,
    mcrota,
    mcio,
    d2ud,
    hooc
};

//! 函数名称字符串的表.
char* cmd_table[NUM_OF_CMD] = 
{
    "quit",
    "lscmd",
    "help",

    "reset",
    "step",
    "servo",
    "mcrota",
    "mcio",
    "d2ud",
    "hooc"
};


int main(int argc, char* argv[])
{
    if(wiringPiSetup() == -1)
    {
        printf(COLOR_BOLD_RED "wiringPi library open failed!" COLOR_OFF "\n");
        return -1;
    }

    fd = serialOpen("/dev/ttyAMA0", 115200);
    if(fd == -1)
    {
        printf(COLOR_BOLD_RED "serial port open failed!" COLOR_OFF "\n");
        return -1;
    }

    //! 报幕lol.
    printf("\n" COLOR_BOLD_BLUE "This is manual debug program! Designed by tuwenbo (2023-4-9)!" COLOR_OFF "\n\n");

    printf("If print info about 'args' and 'index'?(y/n)");
    fflush(stdout);
    char choice = '0';
    scanf("%c", &choice);

    char input_cmd[20];
    char cmd[10], str_arg1[10], str_arg2[10], str_arg3[10];
    uint32_t arg1 = 0U, arg2 = 0U, arg3 = 0U;

    int index = 0;//!< 命令的索引.
    for( ; ; )
    {
        printf(COLOR_BOLD_BLUE ">>> " COLOR_OFF);
        fflush(stdout);

        read(0, input_cmd, 200);//!< 获取到用户输入的字符串, 以'\n'结尾.

        get_cmd_and_args(input_cmd, cmd, str_arg1, str_arg2, str_arg3);//!< 解析得到各自的字符串.
        
        arg1 = str2uint32(str_arg1);//!< 将参数转化为数字.
        arg2 = str2uint32(str_arg2);
        arg3 = str2uint32(str_arg3);

        //! 如果需要打印信息, 就打印.
        if(choice == 'y')
            printf("arg1 = %d\narg2 = %d\narg3 = %d\n", arg1, arg2, arg3);

        for(index = 0; index < NUM_OF_CMD; index++)//!< 匹配命令.
        {
            if(strcmp(cmd_table[index], cmd) == 0)
                break;
        }
        
        if(choice == 'y')
            printf("index = %d\n\n", index);

        if(index > NUM_OF_CMD - 1U)
        {
            printf(COLOR_BOLD_RED "DO NOT have this cmd!\n\n" COLOR_OFF);
            continue;
        }
        else
            cmd_funcs[index](arg1, arg2, arg3);//!< 执行对应的函数.
    }//!< end of 'for( ; ; )'

    return 0;
}


//! 从命令行的输入字符串中解析出cmd字符串和3个参数字符串的函数.
void get_cmd_and_args(char* str, char* cmd, char* arg1, char* arg2, char* arg3)
{
    int i = 0;
    for( ; ; )//!< get cmd.
    {
        if(str[i] != ' ' && str[i] != '\n' && str[i] != '\0')
        {
            cmd[i] = str[i];
            i++;
        }
        else
        {
            cmd[i] = '\0';//!< 先把cmd给安排好.

            if(str[i] == '\n')//!< 如果命令是以'\n'结尾的, 那就说明该命令不需要参数.
            {
                arg1[0] = INVALID_CHAR; arg1[1] = '\0';//!< '1'表示无效值, 代表无效参数.(当调用str2uint32时会生成无效参数1).
                arg2[0] = INVALID_CHAR; arg2[1] = '\0';
                arg3[0] = INVALID_CHAR; arg3[1] = '\0';
                
                return ;//!< 后面就不需要再执行, 直接退出函数.
            }
            i++;
            break;
        }
    }

    int j = 0;
    for( ; ; )//!< get arg1.
    {
        if(str[i] != ' ' && str[i] != '\n')
        {
            arg1[j] = str[i];
            j++;
            i++;
        }
        else
        {
            arg1[j] = '\0';

            if(str[i] == '\n')
            {
                arg2[0] = INVALID_CHAR; arg2[1] = '\0';
                arg3[0] = INVALID_CHAR; arg3[1] = '\0';
                
                return ;//!< 后面就不需要再执行, 直接退出函数.
            }
            
            i++;
            break;
        }    
    }

    int k = 0;
    for( ; ; )//!< get arg2.
    {
        if(str[i] != ' ' && str[i] != '\n')
        {
            arg2[k] = str[i];
            k++;
            i++;
        }
        else
        {
            arg2[k] = '\0';

            if(str[i] == '\n')
            {
                arg3[0] = INVALID_CHAR; arg3[1] = '\0';
                
                return ;//!< 后面就不需要再执行, 直接退出函数.
            }

            i++;
            break;
        }
    }

    int l = 0;
    for( ; ; )//!< get arg3.
    {
        if(str[i] != '\n')
       {
            arg3[l] = str[i];
            l++;
            i++;
        }
        else
        {
            arg3[l] = '\0';
            i++;
            break;
        }
    }
}


//! 将数字字符串转化为数字的函数.
uint32_t str2uint32(char* str)
{
    uint32_t res = 0;
    int i = 0;

    while(str[i] != '\0' && str[i] != '\n' && str[i] != ' ')
    {
        res = res * 10U + (str[i] - '0');
        i++;
    }

    return res;
}

