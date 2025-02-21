#include "../include/ui.h"
#include "../include/send_msg.h"
#include "../include/mks_log.h"
#include "../include/event.h"
#include "../include/app_client.h"
#include "../include/mks_file.h"
#include "../include/mks_preview.h"
#include "../include/mks_update.h"
#include "../include/mks_wpa_cli.h"

#include "../include/MakerbaseNetwork.h"
#include "../include/MakerbaseSerial.h"
#include "../include/MakerbaseWiFi.h"
#include "../include/MakerbaseParseIni.h"

#include "../include/system_setting.h"
#include "../include/online_update.h"

#include <stack>

// #define DEFAULT_DIR "gcodes/sda1/"
#define DEFAULT_DIR "gcodes/"

int tty_fd = -1;

/* wifi */
extern struct mks_wifi_status_result_t status_result;
extern struct mks_wifi_status_t wpa_status;

//B
bool no_error = false;
extern bool is_first_refresh_device_code;
std::string tmp_username = "";
bool is_in_qr_page = false;
bool is_in_user_page = false;
extern std::string bind_code;
extern std::string user_token;
extern int lang_index;

// 补偿值
extern bool fresh_page_set_zoffset_data;

// update
extern bool detected_plus3_soc_data;
extern bool detected_mcu_data;
extern bool detected_plus3_ui_data;

// preview
extern std::vector<std::string> gimage;
extern std::vector<std::string> simage;

extern std::string str_gimage;

// wifi start
extern int page_wifi_ssid_list_pages;
extern int page_wifi_current_pages;
// wifi end

extern int page_files_pages;
extern int page_files_current_pages;
extern int page_files_folder_layers;

extern std::string page_files_list_name[8];                                     // 文件列表显示文件名称
extern std::string page_files_list_show_name[8];                                // 文件列表名称
extern std::string page_files_list_show_type[8];   

extern std::stack<std::string> page_files_path_stack;          // 路径栈
extern std::string page_files_root_path;                       // Klippy根目录
extern std::string page_files_previous_path;                   // 之前的路径
extern std::string page_files_path;                            // 文件所在路径
extern std::string page_files_print_files_path;                // 要打印的文件路径

int current_page_id;                    // 当前页面的id号
int previous_page_id;                   // 上一页面的id号
int next_page_id;                       // 下一页面的id号

int event_id;
int page_id;
int widget_id;
int type_id;

int level_mode;                         // 调平模式

bool show_preview_complete = false;
bool show_preview_gimage_completed = false;

bool printing_keyboard_enabled = false;
// bool filament_keyboard_enabled = false;
bool auto_level_button_enabled = true;
bool manual_level_button_enabled = true;

bool printing_wifi_keyboard_enabled = false;

extern std::string printer_idle_timeout_state;
extern std::string printer_print_stats_state;

// 打印调平
int level_mode_printing_extruder_target = 0;
int level_mode_printing_heater_bed_target = 0;

bool level_mode_printing_is_printing_level = false;

bool page_wifi_list_ssid_button_enabled[5] = {false};

// page print filament
bool page_print_filament_extrude_restract_button = false;
bool page_filament_extrude_button = false;
bool page_filament_unload_button = false;

// page reset
// bool page_reset_to_about = false;
extern std::string mks_page_internet_ip;

extern bool mks_file_parse_finished;

//2023.4.20 打印过文件标红
//std::string printed_file_path;

//2023.4.26-1 修改退料流程
extern int printer_extruder_temperature;
extern int printer_extruder_target;

//2023.5.11 CLL 修复页面跳转bug
extern bool printer_ready;

extern std::string printer_webhooks_state;

//2.1.2 CLL 新增热床调平
bool printer_bed_leveling_state;

//2.1.2 CLL 修复页面跳转bug
extern bool printer_ready;
extern std::string printer_webhooks_state;

//4.2.3 CLL 耗材确认弹窗新增不再提示按钮
bool preview_pop_1_on = true;
bool preview_pop_2_on = true;

//4.2.4 CLL 修复断料检测与退料冲突bug
extern bool filament_switch_sensor_fila_enabled;
extern bool previous_filament_sensor_state;

//4.2.5 CLL 新增息屏功能
bool previous_caselight_value = false;
extern float printer_caselight_value;

//4.2.7 CLL 防止在预览图界面卡死
extern bool jump_to_print;

int load_target;
bool load_mode;

extern int mks_connection_method;
extern bool qr_refreshed;

extern int current_server_page;
extern int total_server_count;

void parse_cmd_msg_from_tjc_screen(char *cmd) {
    event_id = cmd[0];
    MKSLOG_BLUE("#########################%s", cmd);
    MKSLOG_RED("0x%x", cmd[0]);
    MKSLOG_RED("0x%x", cmd[1]);
    MKSLOG_RED("0x%x", cmd[2]);
    MKSLOG_RED("0x%x", cmd[3]);
    switch (event_id)
    {
    case 0x03:
        // MKSLOG_BLUE("读取到屏幕固件数据发生错误，进入到恢复屏幕的模式");
        break;

    case 0x05:
        std::cout << "可以开始发送数据了" << std::endl;
        // set_option(tty_fd, 921600, 8, 'N', 1);
        // set_option(tty_fd, 230400, 8, 'N', 1);
        download_to_screen();
        // 收到0x05消息，意味着可以开始发送串口屏幕的信息
        break;

    case 0x1a:
        std::cout << "变量名称无效" << std::endl;
        break;

    case 0x65:
        page_id = cmd[1];
        widget_id = cmd[2];
        type_id = cmd[3];
        tjc_event_clicked_handler(page_id, widget_id, type_id);
        break;

    case 0x66:
        break;

    case 0x67:
        break;

    case 0x68:
        break;
    
    case 0x70:
        MKSLOG_RED("0x%x", event_id);
        // page_id = cmd[1];
        // widget_id = cmd[2];
        // type_id = cmd[3];
        tjc_event_keyboard(cmd);
        break;

    case 0x71:
        page_id = cmd[1];
        tjc_event_setted_handler(cmd[1], cmd[2], cmd[3], cmd[4]);
        break;

    case 0x86:
    case 0x87:
    case 0x88:
    case 0x89:
        MKSLOG_RED("0x%x", event_id);
        break;

    case 0x91:
        current_page_id = TJC_PAGE_LOGO;
        page_to(TJC_PAGE_UPDATED);
        break;

    //B

    case 0xf9:
        is_in_user_page = true;
        MKSLOG_RED("Enter USER Page");
        break;

    case 0xfa:
        is_in_user_page = false;
        bind_code = "";
        MKSLOG_RED("Leave USER Page");
        if (cmd[1] == 0xf9) {
            is_in_user_page = true;
            MKSLOG_RED("Enter USER Page");
        }
        break;

    case 0xfb:
        is_in_qr_page = true;
        lang_index = cmd[1];
        MKSLOG_RED("0x%x 0x%x 0x%x 0x%x ", cmd[0], cmd[1], cmd[2], cmd[3]);
        MKSLOG_RED("Enter QR Page");
        break;

    case 0xfc:
        is_in_qr_page = false;
        bind_code = "";
        MKSLOG_RED("Leave QR Page");
        break;

    case 0xfd:
        break;
    case 0xfe:
        set_option(tty_fd, 230400, 8, 'N', 1);
        download_to_screen();
        break;

    case 0xff:
        MKSLOG_RED("0x%x", event_id);
        break;
    
    default:
        break;
    }
    cmd = NULL;
}

void page_to(int page_id) {
    previous_page_id = current_page_id;
    current_page_id = page_id;
    send_cmd_page(tty_fd, std::to_string(page_id));
}

void tjc_event_clicked_handler(int page_id, int widget_id, int type_id) {
    std::cout << "+++++++++++++++++++" << page_id << std::endl;
    std::cout << "+++++++++++++++++++" << widget_id << std::endl;
    std::cout << "+++++++++++++++++++" << type_id << std::endl;
    switch (page_id)
    {
    // 开箱第一项
    case TJC_PAGE_OPEN_LANGUAGE:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_LANGUAGE_NEXT:
            page_to(TJC_PAGE_OPEN_VIDEO_1);
            break;
        
        //2023.4.20 开机引导跳过
        case TJC_PAGE_OPEN_LANGUAGE_SKIP:
            //2023.5.12 CLL 开机引导跳过确认弹窗
            page_to(TJC_PAGE_OPEN_POP_1);
            break;

        default:
            break;
        }

        break;

    // 开箱第二项
    case TJC_PAGE_OPEN_VIDEO_1:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_VIDEO_1_RIGHT:
            page_to(TJC_PAGE_OPEN_VIDEO_2);
            break;

        default:
            break;
        }
        break;

    // 开箱第三项
    case TJC_PAGE_OPEN_VIDEO_2:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_VIDEO_2_RIGHT:
            page_to(TJC_PAGE_OPEN_VIDEO_3);
            break;
        
        default:
            break;
        }
        break;

    // 开机第四项
    case TJC_PAGE_OPEN_VIDEO_3:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_VIDEO_3_RIGHT:
            open_heater_bed_up();
            break;
        default:
            break;
        }
        break;
    
    case TJC_PAGE_OPEN_VIDEO_4:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_VIDEO_4_UP: // CLL 新增取出珍珠棉界面上下移动平台选项
            send_gcode("G91\nG0 Z-10 F600\nG90\n");
            break;

        case TJC_PAGE_OPEN_VIDEO_4_DOWN:
            send_gcode("G91\nG0 Z10 F600\nG90\n");
            break;
        
        case TJC_PAGE_OPEN_VIDEO_4_NEXT:
            page_to(TJC_PAGE_FILAMENT_VIDEO_1);
            break;

        default:
            break;
        }
        break;

    // 开机第五项
    // case TJC_PAGE_OPEN_LEVELINIT:
    //     switch (widget_id)
    //     {
    //     default:
    //         break;
    //     }
    //     break;

    // case TJC_PAGE_OPEN_LEVELING:
    //     break;

    // 开机第六项
    case TJC_PAGE_OPEN_LEVEL:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_LEVEL_001:
            set_auto_level_dist(0.01);
            break;

        case TJC_PAGE_OPEN_LEVEL_005:
            set_auto_level_dist(0.05);
            break;

        case TJC_PAGE_OPEN_LEVEL_01:
            set_auto_level_dist(0.1);
            break;

        case TJC_PAGE_OPEN_LEVEL__1:
            set_auto_level_dist(1);
            break;

        case TJC_PAGE_OPEN_LEVEL_UP:
            start_auto_level_dist(false);
            break;

        case TJC_PAGE_OPEN_LEVEL_DOWN:
            start_auto_level_dist(true);
            break;

        case TJC_PAGE_OPEN_LEVEL_ENTER:
            if(printer_idle_timeout_state != "Printing") {
                page_to(TJC_PAGE_OPEN_LEVELING);
                //4.2.4 CLL 修改调平
                save_current_zoffset();
                start_auto_level();
            }
            break;

        default:
            break;
        }
        break;

    // 开机第七项
    case TJC_PAGE_OPEN_LEVELED:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_LEVELED_FINISH:
            //4.2.1 CLL 修复卡在自动调平页面
            //if (auto_level_button_enabled == true) {
                auto_level_button_enabled = false;
                // page_to(TJC_PAGE_OPEN_SYNTONY);
                open_go_to_syntony_move();
            //}
            
            // std::cout << "共振补偿按下按钮" << std::endl;
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_NULL_2:
        switch (widget_id)
        {
        case TJC_PAGE_NULL_2_BACK:
            // page_to(TJC_PAGE_OPEN_VIDEO_3);
            break;

        case TJC_PAGE_NULL_2_ENTER:
            //2023.4.27 防止开机引导卡死
            //2023.4.28 删除"取出垫片"界面
            page_to(TJC_PAGE_OPEN_LEVELINIT);
            open_null_2_enter();
            pre_open_auto_level_init();
            break;

        default:
            break;
        }
        break;

    // 开机第九项
    case TJC_PAGE_FILAMENT_VIDEO_1:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_VIDEO_1_RIGHT:
            //2023.5.13 CLL 防止点击过快报错
            if (printer_idle_timeout_state == "Ready" ||printer_idle_timeout_state == "Idle") {
                open_down_50();
                page_to(TJC_PAGE_FILAMENT_VIDEO_2);
            }
            break;
        default:
            break;
        }
        break;

    // 开机第十项
    case TJC_PAGE_FILAMENT_VIDEO_2:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_VIDEO_2_DECREASE:
            set_filament_extruder_target(false);
            break;
        
        case TJC_PAGE_FILAMENT_VIDEO_2_INCREASE:
            set_filament_extruder_target(true);
            break;

        case TJC_PAGE_FILAMENT_VIDEO_2_RIGHT:
            page_to(TJC_PAGE_FILAMENT_VIDEO_3);
            break;

        case TJC_PAGE_FILAMENT_VIDEO_2_TARGET:
            open_set_print_filament_target();
            break;

        default:
            break;
        }
        break;

    // 开机第十一项
    case TJC_PAGE_FILAMENT_VIDEO_3:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_VIDEO_3_EXTRUDE:
            open_start_extrude();
            break;
        
        case TJC_PAGE_FILAMENT_VIDEO_3_NEXT:
            page_to(TJC_PAGE_MORE_LEVEL);
            break;

        default:
            break;
        }
        break;

    // 开箱第十二项
    case TJC_PAGE_MORE_LEVEL:
        switch (widget_id)
        {
        case TJC_PAGE_MORE_LEVEL_FINISH:
            //2023.5.12 CLL 修复开机引导完成热床仍显示加热bug
            get_object_status();
            open_more_level_finish();
            break;

        case TJC_PAGE_MORE_LEVEL_PRINT:
            // page_to(TJC_PAGE_MORE_LEVEL_PLU);
            break;

        default:
            break;
        }
        break;

    // 开箱第十三项
    case TJC_PAGE_MORE_LEVEL_PLU:
        switch (widget_id)
        {
        case TJC_PAGE_MORE_LEVEL_PLU_BACK:
            page_to(TJC_PAGE_MORE_LEVEL);
            break;

        case TJC_PAGE_MORE_LEVEL_PLU_ENTER:
            page_to(TJC_PAGE_MAIN);
            break;
        }
        break;
    
    case TJC_PAGE_MAIN:
        switch (widget_id)
        {
        case TJC_PAGE_MAIN_BTN_HOME:
            MKSLOG_BLUE("按下Home按钮");
            break;

        case TJC_PAGE_MAIN_BTN_FILE:
            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();
            get_object_status();
            */
            
            // 屏蔽内置文件系统
            //if (detect_disk() == 0) { //2023.5.8 CLL 去除U盘判断条件
            page_to(TJC_PAGE_FILE_LIST_1);
            // printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            // page_files_path = "/sda1";
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();
            get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            break;

        case TJC_PAGE_MAIN_BTN_TOOL:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_MAIN_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;
        
        case TJC_PAGE_MAIN_LIGHT:
            led_on_off();
            break;

        case TJC_PAGE_MAIN_VOICE:
            beep_on_off();
            break;

        case TJC_PAGE_MAIN_LOCK:
            motors_off();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_FILE_LIST_1:
        switch (widget_id)
        {
        case TJC_PAGE_FILE_LIST_1_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_FILE:
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_TOOL:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_SERVICE:
            page_to(TJC_PAGE_SERVICE);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_1:
            get_sub_dir_files_list(0);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_2:
            get_sub_dir_files_list(1);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_3:
            get_sub_dir_files_list(2);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_4:
            get_sub_dir_files_list(3);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_5:
            get_sub_dir_files_list(4);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_6:
            get_sub_dir_files_list(5);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_7:
            get_sub_dir_files_list(6);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_8:
            get_sub_dir_files_list(7);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_PREVIOUS:
            if (page_files_current_pages > 0) {
                // std::cout << "############################" << page_files_current_pages << std::endl;
                // page_files_current_pages = (page_files_current_pages + page_files_pages) % (page_files_pages + 1);
                page_files_current_pages -= 1;
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
            }
            break;
        
        case TJC_PAGE_FILE_LIST_1_BTN_NEXT:
            if (page_files_current_pages < page_files_pages) {
                // std::cout << "############################" << page_files_current_pages << std::endl;
                // page_files_current_pages = (page_files_current_pages + 1) % (page_files_pages + 1);
                page_files_current_pages += 1;
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
            }
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_FILE_LIST_2:
        switch (widget_id)
        {
        case TJC_PAGE_FILE_LIST_2_BTN_1:
            get_sub_dir_files_list(0);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_2:
            get_sub_dir_files_list(1);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_3:
            get_sub_dir_files_list(2);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_4:
            get_sub_dir_files_list(3);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_5:
            get_sub_dir_files_list(4);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_6:
            get_sub_dir_files_list(5);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_7:
            get_sub_dir_files_list(6);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_8:
            get_sub_dir_files_list(7);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_BACK:
            // page_files_folder_layers--;
            get_parenet_dir_files_list();
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_PREVIOUS:
            if (page_files_current_pages > 0) {
                // std::cout << "############################" << page_files_current_pages << std::endl;
                // page_files_current_pages = (page_files_current_pages + page_files_pages) % (page_files_pages + 1);
                page_files_current_pages -= 1;
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_2();
            }
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_NEXT:
            if (page_files_current_pages < page_files_pages) {
                page_files_current_pages += 1;
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_2();
            }
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;
        
        case TJC_PAGE_FILE_LIST_2_BTN_FILE:
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_TOOL:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_SERVICE:
            page_to(TJC_PAGE_SERVICE);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_DELETE_FILE:
        switch (widget_id)
        {
        case TJC_PAGE_DELETE_FILE_YES:
            delete_file(page_files_root_path + page_files_print_files_path);
            get_parenet_dir_files_list();
            clear_page_preview();
            show_preview_complete = false;
            clear_cp0_image();
            break;

        case TJC_PAGE_DELETE_FILE_NO:
            page_to(TJC_PAGE_PREVIEW);
            // clear_page_preview();
            show_preview_complete = false;
            // clear_cp0_image();
            break;
        }
        break;
    
    case TJC_PAGE_PREVIEW:
        switch (widget_id)
        {
        case TJC_PAGE_PREVIEW_BTN_BACK:
            //4.2.7 CLL 防止在预览图界面卡死
            if (printer_print_stats_state == "printing" || printer_print_stats_state == "paused") {
                page_to(TJC_PAGE_PRINTING);
                jump_to_print = false;
            } else if (mks_file_parse_finished == false) {
                get_parenet_dir_files_list();
                clear_page_preview();                   // 返回时清除读取的数据
                show_preview_complete = false;
                // show_preview_gimage_completed = false;
                clear_cp0_image();
            } else {
                if (show_preview_complete == true) {        // 当且仅当预览加载完成才可以按下按钮
                    get_parenet_dir_files_list();
                    clear_page_preview();                   // 返回时清除读取的数据
                    show_preview_complete = false;
                    // show_preview_gimage_completed = false;
                    clear_cp0_image();
                }
            }
            break;

        case TJC_PAGE_PREVIEW_BTN_DELETE:
            if (show_preview_complete == true) {        // 当且仅当预览加载完成才可以按下按钮
                page_to(TJC_PAGE_DELETE_FILE);
                // delete_file(page_files_root_path + page_files_print_files_path);
                // get_parenet_dir_files_list();
                // show_preview_complete = false;
                // show_preview_gimage_completed = false;
                // clear_cp0_image();
            }
            break;

        case TJC_PAGE_PREVIEW_BTN_START:
            std::cout << "文件路径：" << page_files_print_files_path << std::endl;
            //4.3.7 CLL 防止在预览图界面卡死
            if (printer_print_stats_state == "printing" || printer_print_stats_state == "paused") {
                page_to(TJC_PAGE_PRINTING);
                jump_to_print = false;
            } else if (show_preview_complete == true) {        // 当且仅当预览加载完成才可以按下按钮
                //4.3.10 CLL 修改断料检测开关逻辑
                if (get_mks_fila_status() == true) {
                    if (get_filament_detected() == true) {
                        check_filament_type();
                        printer_print_stats_state = "printing";
                        filament_sensor_switch(true);
                        print_start();
                        start_printing(page_files_print_files_path);
                    } else {
                        std::cout << "检测到断料" << std::endl;
                        page_to(TJC_PAGE_PRINT_F_POP);
                    }
                } else {
                    print_start();
                    start_printing(page_files_print_files_path);
                    printer_print_stats_state = "printing";
                    check_filament_type();
                    //page_to(TJC_PAGE_PRINTING);
                }
                printer_ready = false;
                show_preview_complete = false;
            }
            break;
        
        //2.1.2 CLL 新增热床调平
        case TJC_PAGE_PREVIEW_BED_LEVELING:
            if (printer_bed_leveling_state == true) {
                bed_leveling_switch(false);
            }else if(printer_bed_leveling_state == false) {
                bed_leveling_switch(true);
            }

        default:
            break;
        }
        break;

    //2.1.2 CLL 重置打印界面
    case TJC_PAGE_PRINTING:
        switch (widget_id)
        {
        case TJC_PAGE_PRINTING_SET_EXTRUDER:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_HEATER_BED:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_HOT:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_FAN_0:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_FAN_1:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        //2.1.2 CLL 新增fan3
        case TJC_PAGE_PRINTING_SET_FAN_3:
            printing_keyboard_enabled =true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_SPEED:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_FLOW:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_BTN_Z_OFFSET:
            page_to(TJC_PAGE_PRINT_ZOFFSET);
            set_intern_zoffset(0.05);
            clear_page_printing_arg();
            break;

        /*
        case TJC_PAGE_RPINTING_BTN_SHUTDOWN:
            set_printing_shutdown();
            break;
        */
        case TJC_PAGE_PRINTING_BTN_PAUSE_RESUME:
            printer_ready = false;
            set_print_pause();
            clear_page_printing_arg();
            //4.2.2 CLL 优化页面跳转
            page_to(TJC_PAGE_PRINT_FILAMENT);
            //2023.5.11 CLL 修复页面跳转bug
            /*
            if (get_print_pause_resume() == false) {
                set_print_pause();
                page_to(TJC_PAGE_PRINT_FILAMENT);
                clear_page_printing_arg();
            } else {
                set_print_resume();
            }
            */
            break;

        case TJC_PAGE_PRINTING_BTN_STOP:
            page_to(TJC_PAGE_STOP_PRINT);
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_BTN_LIGHT:
            led_on_off();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_PRINT_FILAMENT:
        switch (widget_id)
        {
        case TJC_PAGE_PRINT_FILAMENT_PAUSE_RESUME:
            MKSLOG_BLUE("get_filament_detected_enable: %d", get_filament_detected_enable());
            MKSLOG_BLUE("get_filament_detected: %d", get_filament_detected());
            if (get_filament_detected_enable() == true) {
                if (get_filament_detected() == true) {
                    //4.2.2 CLL 优化页面跳转
                    printer_ready = false;
                    set_print_resume();
                    page_to(TJC_PAGE_PRINTING);
                } else {
                    page_to(TJC_PAGE_PRINT_F_POP);
                }
            } else {
                //4.2.2 CLL 优化页面跳转
                printer_ready = false;
                set_print_resume();
                page_to(TJC_PAGE_PRINTING);
            }
            // if (get_print_pause_resume() == true) {
            
            // }
            break;

        case TJC_PAGE_PRINT_FILAMENT_STOP:
            page_to(TJC_PAGE_STOP_PRINT);
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINT_FILAMENT_TARGET:
            set_print_filament_target();
            break;

        case TJC_PAGE_PRINT_FILAMENT_TARGET_UP:
            set_filament_extruder_target(true);
            break;

        case TJC_PAGE_PRINT_FILAMENT_TARGET_DOWN:
            set_filament_extruder_target(false);
            break;

        case TJC_PAGE_PRINT_FILAMENT_RETRACT:
            printer_idle_timeout_state = "Printing";
            page_print_filament_extrude_restract_button = true;
            // start_retract();
            send_gcode("M603\n");
            break;

        case TJC_PAGE_PRINT_FILAMENT_EXTRUDE:
            printer_idle_timeout_state = "Printing";
            page_print_filament_extrude_restract_button = true;
            set_print_filament_dist(50);
            start_extrude();
            break;

        case TJC_PAGE_PRINT_FILAMENT_10:
            set_print_filament_dist(10);
            break;

        case TJC_PAGE_PRINT_FILAMENT_50:
            set_print_filament_dist(50);
            break;

        case TJC_PAGE_PRINT_FILAMENT_100:
            set_print_filament_dist(100);
            break;
        
        case TJC_PAGE_PRINT_FILAMENT_UNLOAD:
            if (printer_print_stats_state == "paused") {
                load_mode = false;
                page_to(TJC_PAGE_PRE_HEATING_1);
            }
            break;
            
        case TJC_PAGE_PRINT_FILAMENT_LOAD:
            if (printer_print_stats_state == "paused") {
                load_mode = true;
                page_to(TJC_PAGE_PRE_HEATING_1);
            }
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_MOVE:
        switch (widget_id)
        {
        case TJC_PAGE_MOVE_BTN_FILE:
            /*
            set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();
            get_object_status();
            */
            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_MOVE_BTN_HOME:
            // set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_MOVE_BTN_SERVICE:
            // set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_MOVE_FILAMENT:
            // set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_MOVE_LEVEL_MODE:
            // set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_MOVE_NETWORK:
            // set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_NETWORK_SET);
            refresh_lan_model_frpc();
            break;

        case TJC_PAGE_MOVE_01:
            set_move_dist(0.1);
            break;

        case TJC_PAGE_MOVE_1:
            set_move_dist(1.0);
            break;

        case TJC_PAGE_MOVE_10:
            set_move_dist(10.0);
            break;

        case TJC_PAGE_MOVE_X_UP:
            move_x_increase();
            break;

        case TJC_PAGE_MOVE_X_DOWN:
            move_x_decrease();
            break;

        case TJC_PAGE_MOVE_Y_DOWN:
            move_y_increase();
            break;

        case TJC_PAGE_MOVE_Y_UP:
            move_y_decrease();
            break;

        case TJC_PAGE_MOVE_HOME:
            move_home();
            break;

        case TJC_PAGE_MOVE_Z_UP:
            move_z_decrease();
            break;

        case TJC_PAGE_MVOE_Z_DOWN:
            move_z_increase();
            break;

        case TJC_PAGE_MOVE_BTN_STOP:
            // firmware_reset();
            break;

        case TJC_PAGE_MOVE_M84:
            move_motors_off();
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_MOVE_POP_1:
    case TJC_PAGE_MOVE_POP_2:
    case TJC_PAGE_MOVE_POP_3:
        switch (widget_id)
        {
        case TJC_PAGE_MOVE_POP_1_YES:
            move_home();
            page_to(TJC_PAGE_MOVE);
            //4.2.2 CLL 修复显示bug
            set_move_dist(0.1);             // 恢复步长为0.1
            break;

        case TJC_PAGE_MOVE_POP_1_NO:
            page_to(TJC_PAGE_MOVE);
            //4.2.2 CLL 修复显示bug
            set_move_dist(0.1);             // 恢复步长为0.1
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_BTN_FILE:
            page_to(TJC_PAGE_FILE_LIST_1);
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();
            get_object_status();
            break;

        case TJC_PAGE_FILAMENT_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_FILAMENT_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;
        
        case TJC_PAGE_FILAMENT_MOVE:
            // set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_FILAMENT_AUTOLEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_FILAMENT_NETWORK:
            page_to(TJC_PAGE_NETWORK_SET);
            refresh_lan_model_frpc();
            break;

        case TJC_PAGE_FILAMENT_BTN_10:
            set_print_filament_dist(10);
            break;

        case TJC_PAGE_FILAMENT_BTN_50:
            set_print_filament_dist(50);
            break;

        case TJC_PAGE_FILAMENT_BTN_100:
            set_print_filament_dist(100);
            break;

        case TJC_PAGE_FILAMENT_UNLOAD:
            //2023.5.12 CLL 退料前解锁电机
            move_motors_off();
            load_mode = false;
            page_to(TJC_PAGE_PRE_HEATING_1);
            break;

        case TJC_PAGE_FILAMENT_LOAD:
            move_motors_off();
            load_mode = true;
            page_to(TJC_PAGE_PRE_HEATING_1);
            break;

        case TJC_PAGE_FILAMENT_BTN_RETRACT:
            printer_idle_timeout_state = "Printing";
            page_filament_extrude_button = true;
            
            //4.2.7 CLL 修复耗材进出与断料检测冲突
            if (filament_switch_sensor_fila_enabled == true) {
                set_filament_sensor();
                previous_filament_sensor_state = true;
                start_retract();
            } else {
                start_retract();
            }
            break;

        case TJC_PAGE_FILAMENT_BTN_EXTRUDE:
            printer_idle_timeout_state = "Printing";
            page_filament_extrude_button = true;
            
            //4.2.7 CLL 修复耗材进出与断料检测冲突
            if (filament_switch_sensor_fila_enabled == true) {
                set_filament_sensor();
                previous_filament_sensor_state = true;
                start_extrude();
            } else {
                start_extrude();
            }
            break;

        case TJC_PAGE_FILAMENT_BTN_EXTRUDER:
            page_to(TJC_PAGE_KB_FILAMENT_1);
            break;

        case TJC_PAGE_FILAMENT_BTN_HEATER_BED:
            page_to(TJC_PAGE_KB_FILAMENT_1);
            break;

        case TJC_PAGE_FILAMENT_BTN_HOT:
            page_to(TJC_PAGE_KB_FILAMENT_1);
            break;

        case TJC_PAGE_FILAMENT_BTN_FAN_1:
            std::cout << "按下第一个风扇" << std::endl;
            filament_fan0();
            break;

        case TJC_PAGE_FILAMENT_BTN_FAN_2:
            std::cout << "按下第二个风扇" << std::endl;
            filament_fan2();
            break;

        //2.1.2 CLL 新增fan3
        case TJC_PAGE_FILAMENT_BTN_FAN_3:
            std::cout << "按下第三个风扇" << std::endl;
            filament_fan3();
            break;

        case TJC_PAGE_FILAMENT_BTN_FILAMENT_SENSOR:
        // case 0x10:
            set_filament_sensor();
            break;

        case TJC_PAGE_FILAMENT_BTN_STOP:
            firmware_reset();
            break;

        case TJC_PAGE_FILAMENT_EXTRUDER:
            filament_extruder_target();
            break;

        case TJC_PAGE_FILAMENT_HEATER_BED:
            filament_heater_bed_target();
            break;

        case TJC_PAGE_FILAMENT_HOT:
            filament_hot_target();
            break;
        
        }
        break;

    case TJC_PAGE_FILAMENT_POP:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_POP_YES:
            // set_extruder_target(240);
            // 2023.4.22-1 修改退料流程
            /*
            if (previous_page_id == TJC_PAGE_FILAMENT_POP_4) {
                printer_idle_timeout_state = "Printing";
                filament_unload();
                page_to(TJC_PAGE_FILAMENT);
            } else {
                set_extruder_target(280);       // 按照加热要求加热到280度
                page_filament_extrude_button = false;

                page_to(TJC_PAGE_FILAMENT);
            }
            */
            page_filament_extrude_button =false;
            page_filament_unload_button = false;
            page_to(TJC_PAGE_FILAMENT);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT_POP_4:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_POP_4_OK:
            //4.2.4 CLL 修复断料检测与退料冲突bug
            printer_idle_timeout_state = "Printing";
            filament_unload();
            //2.1.2 CLL 新增退料界面
            if (printer_print_stats_state == "paused") {
                send_cmd_pic(tty_fd, "unloading.b[0]", "438");
            } else {
                send_cmd_pic(tty_fd, "unloading.b[0]", "429");
            }
            page_to(TJC_PAGE_UNLOADING);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_LEVEL_MODE:  
        switch (widget_id)
        {
        case TJC_PAGE_LEVEL_MODE_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */
            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
                sub_object_status();
                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_LEVEL_MODE_HOME:
            get_object_status();
            sub_object_status();
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_LEVEL_MODE_SERVICE:
            get_object_status();
            sub_object_status();
            page_to(TJC_PAGE_LANGUAGE);
            break;
            
        case TJC_PAGE_LEVEL_MODE_MOVE:
            get_object_status();
            sub_object_status();
            set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_LEVEL_MODE_FILAMENT:
            get_object_status();
            sub_object_status();
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_LEVEL_MODE_NETWORK:
            get_object_status();
            sub_object_status();
            page_to(TJC_PAGE_NETWORK_SET);
            refresh_lan_model_frpc();
            break;

        case TJC_PAGE_LEVEL_MODE_AUTO_LEVEL:
            //sub_object_status();
            //get_object_status();
            level_mode = TJC_PAGE_AUTO_LEVEL;
            page_to(TJC_PAGE_LEVEL_NULL_1);
            break;

        case TJC_PAGE_LEVEL_MODE_MANUAL_LEVEL:
            sub_object_status();
            get_object_status();
            page_to(TJC_PAGE_LEVELING_NULL_2);
            break;
        
        //2023.4.20 修改调平数据显示
        case TJC_PAGE_LEVEL_MODE_ZOFFSET:
            get_object_status();
            page_to(TJC_PAGE_SET_ZOFFSET);
            break;

        //4.2.5 CLl 新增设置Z轴偏移界面
        case TJC_PAGE_LEVEL_MODE_SET_ZOFFSET:
            level_mode = TJC_PAGE_SET_ZOFFSET;
            page_to(TJC_PAGE_LEVEL_NULL_2);
            send_cmd_vis(tty_fd, "t2", "0");
            break;

        case TJC_PAGE_LEVEL_MODE_SYNTONY_MOVE:
            sub_object_status();
            get_object_status();
            go_to_syntony_move();
            // level_mode = TJC_PAGE_SYNTONY_MOVE;
            // page_to(TJC_PAGE_LEVELING_NULL);
            break;

        case TJC_PAGE_LEVEL_MODE_PID:
            sub_object_status();
            get_object_status();
            // go_to_pid_working();
            page_to(TJC_PAGE_WAIT_POP);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_LEVELING_NULL:
        switch (widget_id)
        {
        case TJC_PAGE_LEVELING_NULL_BTN_BACK:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_LEVELING_NULL_BTN_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */
            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_LEVELING_NULL_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_LEVELING_NULL_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_LEVELING_NULL_ENTER:
            // page_to(TJC_PAGE_LEVELING_INIT);
            switch (level_mode)
            {
            case TJC_PAGE_AUTO_LEVEL:
                std::cout << "进入到这里" << std::endl;
                auto_level_button_enabled = true;
                printer_idle_timeout_state = "Printing";
                pre_auto_level_init();
                break;

            case TJC_PAGE_SET_ZOFFSET:
                page_to(TJC_PAGE_CHANGE_ZOFFSET);
                break;
            
            default:
                break;
            }
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_LEVELING_NULL_2:
        switch (widget_id)
        {
        case TJC_PAGE_LEVELING_NULL_2_ENTER:
            printer_idle_timeout_state = "Printing";
            pre_manual_level_init();
            break;

        case TJC_PAGE_LEVELING_NULL_2_BACK:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_AUTO_LEVEL:
        switch (widget_id)
        {
        case TJC_PAGE_AUTO_LEVEL_001:
            set_auto_level_dist(0.01);
            break;

        case TJC_PAGE_AUTO_LEVEL_0025:
            // set_auto_level_dist(0.025);
            set_auto_level_dist(0.05);
            break;

        case TJC_PAGE_AUTO_LEVEL_005:
            // set_auto_level_dist(0.05);
            set_auto_level_dist(0.1);
            break;

        case TJC_PAGE_AUTO_LEVEL_1:
            set_auto_level_dist(1);
            break;

        case TJC_PAGE_AUTO_LEVEL_UP:
            start_auto_level_dist(false);
            break;

        case TJC_PAGE_AUTO_LEVEL_DOWN:
            start_auto_level_dist(true);
            break;
        
        case TJC_PAGE_AUTO_LEVEL_ENTER:
            if(printer_idle_timeout_state != "Printing") {
                page_to(TJC_PAGE_AUTO_MOVE);
                //4.2.4 CLL 修改调平
                save_current_zoffset();
                start_auto_level();
            }
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_AUTO_FINISH:
        switch (widget_id)
        {
        case TJC_PAGE_AUTO_FINISH_YES:
            //4.2.2 CLL 修复自动调平页面卡住
            auto_level_button_enabled = false;
            std::cout << "自动调平已完成" << std::endl;
            finish_auto_level();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_PRINT_F_POP:
        switch (widget_id)
        {
        case TJC_PAGE_PRINT_F_POP_YES:
            //2.1.2 CLL 在打印设置zoffset界面也会有断料提醒弹窗
            if (previous_page_id == TJC_PAGE_PRINTING || previous_page_id == TJC_PAGE_PRINT_ZOFFSET) {
                if (get_filament_detected() == false) {
                    page_to(TJC_PAGE_PRINT_FILAMENT);
                } else {
                    page_to(TJC_PAGE_PRINT_FILAMENT);
                }
            } else if (previous_page_id == TJC_PAGE_PREVIEW) {
                if (get_filament_detected() == false) {
                    page_to(TJC_PAGE_FILAMENT);
                }
            } else{ //4.2.2 CLL 修复卡在断料界面
                if (get_filament_detected() == false) {
                    page_to(TJC_PAGE_PRINT_FILAMENT);
                } else {
                    page_to(TJC_PAGE_PRINT_FILAMENT);
                }
            }
            break;
        }
        break;

    case TJC_PAGE_SET_ZOFFSET:
        switch (widget_id)
        {
        case TJC_PAGE_SET_ZOFFSET_BACK:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_SET_ZOFFSET_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_SET_ZOFFSET_FILE:
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            break;

        case TJC_PAGE_SET_ZOFFSET_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_SET_ZOFFSET_B1:
            //move_to_certain_position(0);
            break;

        case TJC_PAGE_SET_ZOFFSET_B2:
            //move_to_certain_position(4);
            break;

        case TJC_PAGE_SET_ZOFFSET_B3:
            //move_to_certain_position(8);
            break;

        case TJC_PAGE_SET_ZOFFSET_B4:
            //move_to_certain_position(12);
            break;

        case TJC_PAGE_SET_ZOFFSET_B5:
            //move_to_certain_position(1);
            break;

        case TJC_PAGE_SET_ZOFFSET_B6:
            //move_to_certain_position(5);
            break;

        case TJC_PAGE_SET_ZOFFSET_B7:
            //move_to_certain_position(9);
            break;

        case TJC_PAGE_SET_ZOFFSET_B8:
            //move_to_certain_position(13);
            break;

        case TJC_PAGE_SET_ZOFFSET_B9:
            //move_to_certain_position(2);
            break;

        case TJC_PAGE_SET_ZOFFSET_B10:
            //move_to_certain_position(6);
            break;

        case TJC_PAGE_SET_ZOFFSET_B11:
            //move_to_certain_position(10);
            break;

        case TJC_PAGE_SET_ZOFFSET_B12:
            //move_to_certain_position(14);
            break;

        case TJC_PAGE_SET_ZOFFSET_B13:
            //move_to_certain_position(3);
            break;

        case TJC_PAGE_SET_ZOFFSET_B14:
            //move_to_certain_position(7);
            break;

        case TJC_PAGE_SET_ZOFFSET_B15:
            //move_to_certain_position(11);
            break;

        case TJC_PAGE_SET_ZOFFSET_B16:
            //move_to_certain_position(15);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_CHANGE_ZOFFSET:
        switch (widget_id)
        {
        case TJC_PAGE_CHANGE_ZOFFSET_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */
            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_CHANGE_ZOFFSET_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_CHANGE_ZOFFSET_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_CHANGE_ZOFFSET_BACK:
            page_to(TJC_PAGE_LEVELING_NULL);
            break;

        case TJC_PAGE_CHANGE_ZOFFSET_ENTER:
            pre_set_zoffset_init();
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_WIFI_LIST_2:
        switch (widget_id)
        {
        case TJC_PAGE_WIFI_LIST_2_BTN_FILE:
            //if (0 == detect_disk()) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();

                mks_wpa_cli_close_connection();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            break;
        
        case TJC_PAGE_WIFI_LIST_2_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            mks_wpa_cli_close_connection();
            break;

        case TJC_PAGE_WIFI_LIST_2_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            mks_wpa_cli_close_connection();
            break;

        case TJC_PAGE_WIFI_LIST_2_MOVE:
            page_to(TJC_PAGE_MOVE);
            mks_wpa_cli_close_connection();
            break;

        case TJC_PAGE_WIFI_LIST_2_FILAMENT:
            page_to(TJC_PAGE_FILAMENT);
            mks_wpa_cli_close_connection();
            break;

        case TJC_PAGE_WIFI_LIST_2_AUTO_LEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            mks_wpa_cli_close_connection();
            break;

        case TJC_PAGE_WIFI_LIST_2_PREVIOUS:
            if (page_wifi_current_pages > 0) {
                std::cout << "page_wifi_current_pages = " << page_wifi_current_pages << std::endl;
                std::cout << "page_wifi_ssid_list_pages = " << page_wifi_ssid_list_pages << std::endl;
                page_wifi_current_pages -= 1;
                set_page_wifi_ssid_list(page_wifi_current_pages);
                refresh_page_wifi_list();
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_NEXT:
            if (page_wifi_current_pages < page_wifi_ssid_list_pages - 1) {
                std::cout << "page_wifi_current_pages = " << page_wifi_current_pages << std::endl;
                std::cout << "page_wifi_ssid_list_pages = " << page_wifi_ssid_list_pages << std::endl;
                page_wifi_current_pages += 1;
                set_page_wifi_ssid_list(page_wifi_current_pages);
                refresh_page_wifi_list();
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_1:
            if (page_wifi_list_ssid_button_enabled[0] == true) {
                get_wifi_list_ssid(0);
                printing_wifi_keyboard_enabled = true;
                page_to(TJC_PAGE_WIFI_KEYBOARD);
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_2:
            if (true == page_wifi_list_ssid_button_enabled[1]) {
                get_wifi_list_ssid(1);
                printing_wifi_keyboard_enabled = true;
                page_to(TJC_PAGE_WIFI_KEYBOARD);
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_3:
            if (true == page_wifi_list_ssid_button_enabled[2]) {
                get_wifi_list_ssid(2);
                printing_wifi_keyboard_enabled = true;
                page_to(TJC_PAGE_WIFI_KEYBOARD);
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_4:
            if (true == page_wifi_list_ssid_button_enabled[3]) {
                get_wifi_list_ssid(3);
                printing_wifi_keyboard_enabled = true;
                page_to(TJC_PAGE_WIFI_KEYBOARD);
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_5:
            if (true == page_wifi_list_ssid_button_enabled[4]) {
                get_wifi_list_ssid(4);
                printing_wifi_keyboard_enabled = true;
                page_to(TJC_PAGE_WIFI_KEYBOARD);
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_REFRESH:
            std::cout << "################## 按下刷新按钮" << std::endl;
            scan_ssid_and_show();
            break;

        case TJC_PAGE_WIFI_LIST_2_EHTNET:
            set_mks_net_eth();
            mks_page_internet_ip = get_eth0_ip();
            page_to(TJC_PAGE_INTERNET);
            break;

        case TJC_PAGE_WIFI_LIST_2_BACK:
            page_to(TJC_PAGE_NETWORK_SET);
            refresh_lan_model_frpc();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_KEYDBA:
        switch (widget_id)
        {
        case TJC_PAGE_KEYDBA_HOME:
            break;

        case TJC_PAGE_KEYDBA_FILE:
            break;

        case TJC_PAGE_KEYDBA_SERVICE:
            break;

        // case TJC_PAGE_KEYDBA_ENTER:
            // break;

        case TJC_PAGE_KEYDBA_BACK:
            printing_wifi_keyboard_enabled = false;
            page_to(TJC_PAGE_WIFI_LIST_2);
            break;

        default:
            break;
        }
        break;

    /*
    case TJC_PAGE_WIFI_LIST_2:
        switch (widget_id)
        {
        case TJC_PAGE_WIFI_LIST_2_BTN_FILE:
            page_to(TJC_PAGE_FILE_LIST_1);
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            break;
        
        case TJC_PAGE_WIFI_LIST_2_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_WIFI_LIST_2_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_WIFI_LIST_2_MOVE:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_WIFI_LIST_2_FILAMENT:
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_WIFI_LIST_2_AUTO_LEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        default:
            break;

        }
        break;
    */

    case TJC_PAGE_INTERNET:
        switch (widget_id)
        {
        case TJC_PAGE_INTERNET_MOVE:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_INTERNET_FILAMENT:
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_INTERNET_AUTO_LEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;
        
        case TJC_PAGE_INTERNET_BTN_FILE:
            //if (0 == detect_disk()) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            break;
        
        case TJC_PAGE_INTERNET_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_INTERNET_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_INTERNET_WIFI:
            page_to(TJC_PAGE_NETWORK_SET);
            refresh_lan_model_frpc();
            break;
        
        default:
            break;

        }
        break;

    case TJC_PAGE_LANGUAGE:
        switch (widget_id)
        {
        case TJC_PAGE_LANGUAGE_BTN_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */

            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;
        
        case TJC_PAGE_LANGUAGE_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_LANGUAGE_BTN_TOOL:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_LANGUAGE_SERVICE:
            page_to(TJC_PAGE_SERVICE);
            break;

        case TJC_PAGE_LANGUAGE_RESET:
            go_to_reset();
            break;

        case TJC_PAGE_LANGUAGE_ABOUT:
            // page_to(TJC_PAGE_ABOUT);
            go_to_about();
            break;

        }
        break;

    //4.2.1 CLL 优化页面跳转
    case TJC_PAGE_SERVICE:
        switch (widget_id)
        {
        case TJC_PAGE_SERVICE_BTN_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */

            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            }
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;
        
        case TJC_PAGE_SERVICE_BTN_HOME:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                get_object_status();
                page_to(TJC_PAGE_MAIN);
            }
            break;

        case TJC_PAGE_SERVICE_BTN_TOOL:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                get_object_status();
                page_to(TJC_PAGE_MOVE);
            }
            break;

        case TJC_PAGE_SERVICE_LANGUAGE:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                get_object_status();
                page_to(TJC_PAGE_LANGUAGE);
            }
            break;

        case TJC_PAGE_SERVICE_RESET:
            get_object_status();
            go_to_reset();
            break;

        case TJC_PAGE_SERVICE_ABOUT:
            // page_to(TJC_PAGE_ABOUT);
            get_object_status();
            go_to_about();
            break;

        }
        break;

    //4.2.1 CLL 优化页面跳转
    case TJC_PAGE_RESET:
        switch (widget_id)
        {
        case TJC_PAGE_RESET_BTN_FILE:
            // page_to(TJC_PAGE_FILE_LIST_1);
            // page_files_pages = 0;
            // page_files_current_pages = 0;
            // page_files_folder_layers = 0;
            // page_files_previous_path = "";
            // page_files_root_path = DEFAULT_DIR;
            // page_files_path = "";
            // refresh_page_files(page_files_current_pages);
            // refresh_page_files_list_1();
            break;
        
        case TJC_PAGE_RESET_BTN_HOME:
            // page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_RESET_BTN_TOOL:
            // page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_RESET_LANGUAGE:
            // page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_RESET_SERVICE:
            page_to(TJC_PAGE_SERVICE);
            break;

        case TJC_PAGE_RESET_ABOUT:
            // page_reset_to_about = true;
            //current_page_id = TJC_PAGE_ABOUT;
            go_to_about();
            break;

        case TJC_PAGE_RESET_RESTART_KLIPPER:
            reset_klipper();
            break;

        case TJC_PAGE_RESET_FIRMWARE_RESTART:
            reset_firmware();
            break;

        }
        break;

    //2023.4.25-4 修复重启后温度显示不正确
    case TJC_PAGE_SYS_OK:
        switch (widget_id)
        {
        case TJC_PAGE_SYS_OK_HOME:
            get_object_status();
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_SYS_OK_FILE:
            get_object_status();
            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */
            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_SYS_OK_TOOL:
            get_object_status();
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_SYS_OK_LANGUAGE:
            get_object_status();
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_SYS_OK_SERVICE:
            get_object_status();
            page_to(TJC_PAGE_SERVICE);
            break;

        case TJC_PAGE_SYS_OK_ABOUT:
            get_object_status();
            // page_to(TJC_PAGE_ABOUT);
            go_to_about();
            break;
        
        default:
            break;
        }
        break;

    // 2023.5.11 CLL 修复页面跳转bug
    case TJC_PAGE_ABOUT:
        switch (widget_id)
        {
        //2023.4.25-1 隐藏开机引导
        case TJC_PAGE_ABOUT_S_BTN:
            set_mks_oobe_enabled(true);
            break;

        case TJC_PAGE_ABOUT_BTN_FILE:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_FILE_LIST_1);
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
                get_object_status();
            }
            break;
        
        case TJC_PAGE_ABOUT_BTN_HOME:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_MAIN);
                get_object_status();
            }
            break;

        case TJC_PAGE_ABOUT_BTN_TOOL:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_MOVE);
                get_object_status();
            }
            break;

        case TJC_PAGE_ABOUT_LANGUAGE:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_LANGUAGE);
                get_object_status();
            }
            break;

        case TJC_PAGE_ABOUT_SERVICE:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_SERVICE);
                get_object_status();
            }
            break;

        case TJC_PAGE_ABOUT_RESET:
            go_to_reset();
            break;

        case TJC_PAGE_ABOUT_OFFLINE_UPDATE:
            if (detect_update())
            {
                page_to(TJC_PAGE_OFF_UPDATE_CHOOSE);
            } else {
                page_to(TJC_PAGE_NO_OFFLINE_FILES);
            }
            break;

        case TJC_PAGE_ABOUT_OOBE:
            std::cout << "打开oobe开关" << std::endl;
            if (get_mks_oobe_enabled() == true) {
                set_mks_oobe_enabled(false);
                send_cmd_picc(tty_fd, "b1", "132");
                send_cmd_vis(tty_fd, "b1", "0");
            } else {
                set_mks_oobe_enabled(true);
                send_cmd_picc(tty_fd, "b1", "367");
                send_cmd_vis(tty_fd, "b1", "1");
            }
            break;

        //4.2.7 CLL 新增恢复出厂设置功能
        case TJC_PAGE_ABOUT_RESTORE:
            page_to(TJC_PAGE_RESTORE_CONFIG);
            break;

        //4.2.10 CLL 新增输出日志功能
        case  TJC_PAGE_ABOUT_PRINT_LOG:
            print_log();
            break;

        case TJC_PAGE_ABOUT_ONLINE_UPDATE:
            btn_online_update_click_handler();
            break;

        case TJC_PAGE_ABOUT_SLEEP:
            go_to_about();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_PRINT_ZOFFSET:
        switch (widget_id)
        {
        case TJC_PAGE_PRINT_ZOFFSET_001:
            // set_intern_zoffset(0.005);
            set_intern_zoffset(0.01);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_01:
            // set_intern_zoffset(0.025);
            set_intern_zoffset(0.1);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_005:
            // set_intern_zoffset(0.01);
            set_intern_zoffset(0.05);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_1:
            // set_intern_zoffset(0.05);
            set_intern_zoffset(1);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_UP:
            set_zoffset(false);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_DOWN:
            set_zoffset(true);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_PAUSE_RESUME:
            // 只要暂停就跳到换料页面
            //2023.5.11 CLL 修复页面跳转bug
            printer_ready = false;
            set_print_pause();
            //4.2.2 CLL 优化页面跳转
            page_to(TJC_PAGE_PRINT_FILAMENT);
            clear_page_printing_arg();
            /*
            if (get_print_pause_resume() == false) {
                set_print_pause();
                // page_to(TJC_PAGE_PRINT_FILAMENT);
            } else {
                set_print_resume();
            }
            */
            break;

        case TJC_PAGE_PRINT_ZOFFSET_STOP:
            page_to(TJC_PAGE_STOP_PRINT);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_BACK:
            printing_keyboard_enabled = false;
            page_to(TJC_PAGE_PRINTING);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_PRINT_KEYBOARD:
        switch (widget_id)
        {
        case TJC_PAGE_PRINT_KEYBOARD_BACK:
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            break;
        }
        break;

    case TJC_PAGE_PRINT_FINISH:
        switch (widget_id)
        {
        case TJC_PAGE_PRINT_FINISH_YES:
            finish_print();
            page_to(TJC_PAGE_MAIN);
            break;
        }
        break;

    case TJC_PAGE_STOP_PRINT:
        switch (widget_id)
        {
        case TJC_PAGE_STOP_PRINT_YES:
            //4.2.3 CLL 修复在打印暂停界面取消打印后仍显示加热
            printer_idle_timeout_state = "Printing";
            page_to(TJC_PAGE_STOPPING);
            cancel_print();
            break;

        case TJC_PAGE_STOP_PRINT_NO:
            //4.2.7 CLL 修复页面跳转bug
            page_to(previous_page_id);
            break;
        }
        break;

    //4.2.5 CLL 新增设置Z轴偏移界面
    case TJC_PAGE_SET_ZOFFSET_2:
        switch (widget_id)
        {
        case TJC_PAGE_SET_ZOFFSET_2_001:
            set_auto_level_dist(0.01);
            break;

        case TJC_PAGE_SET_ZOFFSET_2_005:
            set_auto_level_dist(0.05);
            break;

        case TJC_PAGE_SET_ZOFFSET_2_01:
            set_auto_level_dist(0.1);
            break;

        case TJC_PAGE_SET_ZOFFSET_2_1:
            set_auto_level_dist(1);
            break;

        case TJC_PAGE_SET_ZOFFSET_2_DOWN:
            start_auto_level_dist(true);
            break;

        case TJC_PAGE_SET_ZOFFSET_2_UP:
            start_auto_level_dist(false);
            break;

        case TJC_PAGE_SET_ZOFFSET_2_YES:
            save_current_zoffset();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_KEYBDB:
        switch (widget_id)
        {
        case TJC_PAGE_KEYBDB_PAUSE:
            if (get_print_pause_resume() == false) {
                set_print_pause();
                page_to(TJC_PAGE_PRINT_FILAMENT);
                clear_page_printing_arg();
            } else {
                set_print_resume();
            }
            break;

        case TJC_PAGE_KEYBDB_STOP:
            page_to(TJC_PAGE_STOP_PRINT);
            clear_page_printing_arg();
            break;

        case TJC_PAGE_KEYBDB_BACK:
            printing_keyboard_enabled = false;
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_POP_1:
        switch (widget_id)
        {
        case TJC_PAGE_POP_1_YES:
            if (previous_page_id == TJC_PAGE_MOVE) {
                // set_move_dist(0.1);
                page_to(TJC_PAGE_MOVE);
            } else if (previous_page_id == TJC_PAGE_AUTO_LEVEL) {
                std::cout << "按下确定键" << std::endl;
                page_to(TJC_PAGE_MAIN);
            } else if (previous_page_id == TJC_PAGE_PRINTING || current_page_id == TJC_PAGE_PRINT_ZOFFSET || current_page_id == TJC_PAGE_PRINT_FILAMENT) {
                page_to(TJC_PAGE_MAIN);
            } else {
                std::cout << "TJC_PAGE_POP_1_YES" << std::endl;
                page_to(previous_page_id);
            }
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_KB_FILAMENT_1:
    // case TJC_PAGE_KB_FILAMENT_2:
    // case TJC_PAGE_KB_FILAMENT_3:
        switch (widget_id)
        {
        case TJC_PAGE_KB_FILAMENT_1_BACK:
            page_to(TJC_PAGE_FILAMENT);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_DISK_DETECT:
        switch (widget_id)
        {
        case TJC_PAGE_DISK_DETECT_ENTER:
            page_to(previous_page_id);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_UPDATED:
        switch (widget_id)
        {
        case TJC_PAGE_UPDATED_ENTER:
            if (previous_page_id == TJC_PAGE_LOGO) {
                finish_tjc_update();
                page_to(TJC_PAGE_MAIN);
            }
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_SAVING:
        break;

    case TJC_PAGE_SET_TEMP_LEVEL:
        switch (widget_id)
        {
        case TJC_PAGE_SET_TEMP_LEVEL_HOME:
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_FILE:
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_TOOL:
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_SERVICE:
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_BACK:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_EXTRUDER:
            page_to(TJC_PAGE_SET_TEMP_LEVEL_KB);
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_HOT_BED:
            page_to(TJC_PAGE_SET_TEMP_LEVEL_KB);
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_ENTER:
            level_mode_printing_set_target();
            level_mode_printing_is_printing_level = true;
            printer_print_stats_state = "printing";
            level_mode_printing_print_file();
            page_to(TJC_PAGE_LEVEL_PRINT);
            break;

        case TJC_PAGE_LEVEL_PRINT:
            break;

        case TJC_PAGE_LEVEL_STOP:
            break;

        case TJC_PAGE_CHOOSE_MODEL:
            switch (widget_id)
            {
            case TJC_PAGE_CHOOSE_MODEL_01:
                std::cout << "选择了第1个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_02:
                std::cout << "选择了第2个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_03:
                std::cout << "选择了第3个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_04:
                std::cout << "选择了第4个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_05:
                std::cout << "选择了第5个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_06:
                std::cout << "选择了第6个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_07:
                std::cout << "选择了第7个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_08:
                std::cout << "选择了第8个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_09:
                std::cout << "选择了第9个 zoffset" << std::endl;
                break;

            default:
                break;
            }
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_PID_WORKING:
        break;

    case TJC_PAGE_PID_FINISH:
        break;

    case TJC_PAGE_DISK_DETECT_2:
        switch (widget_id)
        {
        case TJC_PAGE_DISK_DETECT_2_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_DISK_DETECT_2_TOOL:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_DISK_DETECT_2_SERVICE:
            page_to(TJC_PAGE_SERVICE);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_WIFI_SUCCESS:
        switch (widget_id)
        {
        case TJC_PAGE_WIFI_SUCCESS_YES:
            wifi_save_config();
            // page_to(TJC_PAGE_WIFI_SAVE);
            //4.2.3 CLL 修复WiFi刷新bug
            system("dhcpcd wlan0");
            mks_wpa_cli_close_connection();
            scan_ssid_and_show();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_WIFI_FAILED:
        switch (widget_id)
        {
        case TJC_PAGE_WIFI_FAILED_YES:
            page_to(TJC_PAGE_WIFI_LIST_2);
			mks_wpa_cli_close_connection();
			scan_ssid_and_show();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_WIFI_KEYBOARD:
        switch (widget_id)
        {
        case TJC_PAGE_WIFI_KEYBOARD_BACK:
            page_to(TJC_PAGE_WIFI_LIST_2);
            printing_wifi_keyboard_enabled = false;
            //4.2.3 CLL 修复WiFi刷新bug
            set_page_wifi_ssid_list(page_wifi_current_pages);
            refresh_page_wifi_list();
            break;

        case TJC_PAGE_WIFI_KEYBOARD_ENTER:
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_WAIT_POP:
        switch (widget_id)
        {
        case TJC_PAGE_WAIT_POP_OK:
            page_to(previous_page_id);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT_POP_3:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_POP_3_OK:
            page_to(TJC_PAGE_PRINT_FILAMENT);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT_POP_5:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_POP_5_OK:
            page_to(TJC_PAGE_FILAMENT);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT_SHOWDOWN:
        switch (widget_id)
        {
        default:
            break;
        }
        break;

    //2023.5.8 CLL 修改自动调平页面跳转
    case TJC_PAGE_LEVEL_NULL_1:
        switch (widget_id)
        {
        case TJC_PAGE_LEVEL_NULL_1_DOWN:
            set_auto_level_heater_bed_target(false);
            break;
        
        case TJC_PAGE_LEVEL_NULL_1_UP:
            set_auto_level_heater_bed_target(true);
            break;

        case TJC_PAGE_LEVEL_NULL_1_ON_OFF:
            filament_heater_bed_target();
            break;

        case TJC_PAGE_LEVEL_NULL_1_BACK:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_LEVEL_NULL_1_ENTER:
            page_to(TJC_PAGE_LEVEL_NULL_2);
            break;

        default:
            break;
        }
        break;

    //2023.5.8 CLL 修改自动调平页面跳转
    case TJC_PAGE_LEVEL_NULL_2:
        switch(widget_id)
        {
        case TJC_PAGE_LEVEL_NULL_2_BACK:
            if (level_mode == TJC_PAGE_SET_ZOFFSET) {
                page_to(TJC_PAGE_LEVEL_MODE);
            } else {
                page_to(TJC_PAGE_LEVEL_NULL_1);
            }
            break;

        case TJC_PAGE_LEVEL_NULL_2_ENTER:
            if (printer_webhooks_state == "ready") {
                std::cout << "进入到这里" << std::endl;
                if (level_mode == TJC_PAGE_SET_ZOFFSET) {
                    printer_idle_timeout_state = "Printing";
                    pre_auto_level_init();
                } else {
                    auto_level_button_enabled = true;
                    printer_idle_timeout_state = "Printing";
                    pre_auto_level_init();
                }
            }
            break;

        default:
            break;
        }
        break;

    //2023.5.8 CLL 报错弹窗
    case TJC_PAGE_DETECT_ERROR:
        switch(widget_id)
        {
        case TJC_PAGE_DETECT_ERROR_YES:
            page_to(TJC_PAGE_MAIN);
            break;

        default:
            break;
        }
        break;
    
    //2023.5.8 CLL 报错弹窗
    case TJC_PAGE_GCODE_ERROR:
        switch(widget_id)
        {
        case TJC_PAGE_GCODE_ERROR_YES:
            page_to(TJC_PAGE_STOPPING);
            cancel_print();
            break;
        
        default:
            break;
        }
        break;

    //2023.5.11 CLL 修改开机引导自动调平页面跳转
    case TJC_PAGE_OPEN_HEATER_BED:
        switch(widget_id)
        {
        case TJC_PAGE_OPEN_HEATER_BED_DOWN:
            set_auto_level_heater_bed_target(false);
            break;
        
        case TJC_PAGE_OPEN_HEATER_BED_UP:
            set_auto_level_heater_bed_target(true);
            break;

        case TJC_PAGE_OPEN_HEATER_BED_ON_OFF:
            filament_heater_bed_target();
            break;

        case TJC_PAGE_OPEN_HEATER_BED_NEXT:
            page_to(TJC_PAGE_NULL_2);
            break;

        default:
            break;
        }
        break;

    //2023.5.12 CLL 开机引导跳过确认弹窗
    case TJC_PAGE_OPEN_POP_1:
        switch(widget_id)
        {
        case TJC_PAGE_OPEN_POP_1_NO:
            page_to(TJC_PAGE_OPEN_LANGUAGE);
            break;
        
        case TJC_PAGE_OPEN_POP_1_YES:
            page_to(TJC_PAGE_MAIN);
            set_mks_oobe_enabled(false);
            break;
        
        default:
            break;
        }
        break;

    //2.1.2 CLL 打印前判断耗材种类并弹窗
    case TJC_PAGE_PREVIEW_POP_1:
    case TJC_PAGE_PREVIEW_POP_2:
        switch(widget_id)
        {
        case TJC_PAGE_PREVIEW_POP_YES:
            page_to(TJC_PAGE_PRINTING);
            break;
        
        //4.2.3 CLL 耗材确认弹窗新增不再提示按钮
        case TJC_PAGE_PREVIEW_POP_NO_POP:
            if (current_page_id == TJC_PAGE_PREVIEW_POP_1) {
                preview_pop_1_on = false;
            } else if (current_page_id == TJC_PAGE_PREVIEW_POP_2) {
                preview_pop_2_on = false;
            }
            page_to(TJC_PAGE_PRINTING);
            break;
        
        default:
            break;
        }
        break;
    
    //4.2.5 CLL 新增息屏功能
    case TJC_PAGE_SCREEN_SLEEP:
        switch (widget_id)
        {
        case TJC_PAGE_SCREEN_SLEEP_ENTER:
            page_to(TJC_PAGE_SCREEN_SLEEP);
            if (printer_caselight_value == 1) {
                previous_caselight_value = true;
                led_on_off();
            } else {
                previous_caselight_value =false;
            }
            break;

        case TJC_PAGE_SCREEN_SLEEP_EXIT:
            if (previous_page_id == TJC_PAGE_FILE_LIST_1) {
                page_to(previous_page_id);
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
                get_object_status();
            } else if (previous_page_id == TJC_PAGE_FILE_LIST_2) {
                page_to(previous_page_id);
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_2();
                get_object_status();
            } else {
                page_to(previous_page_id);
                get_object_status();
            }
            if (previous_caselight_value == true) {
                led_on_off();
                previous_caselight_value = false;
            }
            break;
        
        default:
            break;
        }
        break;

    //4.2.7 CLL 新增恢复出厂设置功能
    case TJC_PAGE_RESTORE_CONFIG:
        switch(widget_id)
        {
        case TJC_PAGE_RESTORE_CONFIG_YES:
            restore_config();
            break;

        case TJC_PAGE_RESTORE_CONFIG_NO:
            go_to_about();
            break;

        default:
            break;
        }
        break;

    //4.2.10 CLL 新增输出日志功能
    case TJC_PAGE_PRINT_LOG_S:
    case TJC_PAGE_PRINT_LOG_F:
        switch(widget_id)
        {
        case TJC_PAGE_PRINT_LOG_YES:
            go_to_about();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_OPEN_SYNTONY:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_SYNTONY_JUMP_OUT:
            send_gcode("SAVE_CONFIG\n");
            page_to(TJC_PAGE_SYNTONY_FINISH);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_SYNTONY_MOVE:
        switch (widget_id)
        {
        case TJC_PAGE_SYNTONY_MOVE_JUMP_OUT:
            send_gcode("SAVE_CONFIG\n");
            page_to(TJC_PAGE_SYNTONY_FINISH);
            break;

        default:
            break;
        }   
        break;

    case TJC_PAGE_LOAD_FINISH:
        switch (widget_id)
        {
        case TJC_PAGE_LOAD_FINISH_YES:
            if (printer_print_stats_state == "paused") {
                page_to(TJC_PAGE_PRINT_FILAMENT);
            } else {
                page_to(TJC_PAGE_FILAMENT);
            }
            set_extruder_target(0);
            break;

        case TJC_PAGE_LOAD_FINISH_RETRY:
            printer_idle_timeout_state = "Printing";
            filament_load();
            page_to(TJC_PAGE_LOADING);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_PRE_HEATING_1:
        switch (widget_id)
        {
        case TJC_PAGE_PRE_HEATING_1_SET_1:
            load_target = 220;
            if (load_mode == true) {
                page_to(TJC_PAGE_PRE_LOAD);
            } else {
                page_to(TJC_PAGE_PRE_UNLOAD);
            }
            break;

        case TJC_PAGE_PRE_HEATING_1_SET_2:
            load_target = 250;
            if (load_mode == true) {
                page_to(TJC_PAGE_PRE_LOAD);
            } else {
                page_to(TJC_PAGE_PRE_UNLOAD);
            }
            break;

        case TJC_PAGE_PRE_HEATING_1_SET_3:
            load_target = 300;
            if (load_mode == true) {
                page_to(TJC_PAGE_PRE_LOAD);
            } else {
                page_to(TJC_PAGE_PRE_UNLOAD);
            }
            break;

        case TJC_PAGE_PRE_HEATING_1_BACK:
            if (printer_print_stats_state == "paused") {
                page_to(TJC_PAGE_PRINT_FILAMENT);
            } else {
                page_to(TJC_PAGE_FILAMENT);
            }
            break;
            
        default:
            break;
        }
        break;

    case TJC_PAGE_PRE_UNLOAD:
        switch (widget_id)
        {
        case TJC_PAGE_PRE_UNLOAD_NEXT:
            printer_idle_timeout_state = "Printing";
            filament_unload();
            page_to(TJC_PAGE_PRE_HEATING_2);
            break;

        case TJC_PAGE_PRE_UNLOAD_BACK:
            page_to(TJC_PAGE_PRE_HEATING_1);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_MEMORY_WARNING:
        switch (widget_id)
        {
        case TJC_PAGE_MEMORY_WARNING_YES:
            page_to(TJC_PAGE_MAIN);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_PRE_LOAD:
        switch (widget_id)
        {
        case TJC_PAGE_PRE_LOAD_NEXT:
            printer_idle_timeout_state = "Printing";
            filament_load();
            page_to(TJC_PAGE_PRE_HEATING_2);
            break;

        case TJC_PAGE_PRE_LOAD_BACK:
            page_to(TJC_PAGE_PRE_HEATING_1);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_UNLOAD_FINISH:
        switch (widget_id)
        {
        case TJC_PAGE_UNLOAD_FINISH_YES:
            set_extruder_target(0);
            if (printer_print_stats_state == "paused") {
                page_to(TJC_PAGE_PRINT_FILAMENT);
            } else {
                page_to(TJC_PAGE_FILAMENT);
            }
            break;

        case TJC_PAGE_UNLOAD_FINISH_RELOAD:
            printer_idle_timeout_state = "Printing";
            load_mode = true;
            page_to(TJC_PAGE_PRE_HEATING_1);
            break;
        
        default:
            break;
        }
        break;
    
    case TJC_PAGE_NETWORK_SET:
        switch (widget_id)
        {
        case TJC_PAGE_NETWORK_SET_MOVE:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_NETWORK_SET_FILAMENT:
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_NETWORK_SET_AUTO_LEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;
        
        case TJC_PAGE_NETWORK_SET_BTN_FILE:
            page_to(TJC_PAGE_FILE_LIST_1);
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();
            get_object_status();
            break;

        case TJC_PAGE_NETWORK_SET_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_NETWORK_SET_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_NETWORK_SET_ETHERNET:
            mksini_update("mks_ethernet", "enable", std::to_string(!sys_set.ethernet));
            sys_set.ethernet = !sys_set.ethernet;
            if (sys_set.ethernet == 0) {
                send_cmd_txt(tty_fd, "t0", status_result.ip_address);
                send_cmd_picc(tty_fd, "b0", "454");
                send_cmd_picc2(tty_fd, "b0", "453");
                send_cmd_pic(tty_fd, "b[0]", "453");
            } else {
                std::string local_ip = get_eth0_ip();
                if (local_ip.find(":") != -1)
                {
                    send_cmd_txt(tty_fd, "t0", "");
                } else {
                    send_cmd_txt(tty_fd, "t0", local_ip);
                }
                send_cmd_picc(tty_fd, "b0", "473");
                send_cmd_picc2(tty_fd, "b0", "472");
                send_cmd_pic(tty_fd, "b[0]", "472");
            }
            break;

        case TJC_PAGE_NETWORK_SET_REFRESH:
            refresh_device_code();
            break;

        case TJC_PAGE_NETWORK_SET_WIFI_LINK:
            go_to_network();
            break;

        case TJC_PAGE_NETWORK_SET_SERVER:
            go_to_server_set(0);
            break;

        case TJC_PAGE_NETWORK_SET_QIDI_LINK:
            if(user_token ==""){
                page_to(TJC_PAGE_QIDI_LINK);
                send_cmd_vis(tty_fd, "gm0", "1");
            }else{
                page_to(TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS);
                mksini_load();
                tmp_username = mksini_getstring("app", "username", "");
                mksini_free();
                send_cmd_txt(tty_fd,"t1",tmp_username);
                login_successed_page();
            }
            break;

        case TJC_PAGE_NETWORK_SET_BTN_LAN:
            if (sys_set.internet_enabled)
            {
                send_cmd_vis(tty_fd, "gm0", "0");
                send_cmd_tsw(tty_fd, "b1", "0");
                send_cmd_tsw(tty_fd, "b3", "0");
                send_cmd_tsw(tty_fd, "b4", "0");
                send_cmd_picc(tty_fd, "b1", "454");
                send_cmd_picc(tty_fd, "b3", "454");
                send_cmd_picc(tty_fd, "b4", "454");
                send_cmd_picc(tty_fd, "b5", "456");
                send_cmd_picc2(tty_fd, "b5", "455");
                send_cmd_pco(tty_fd, "b3", "33808");
                send_cmd_pco(tty_fd, "b4", "33808");
                send_cmd_pco(tty_fd, "t1", "33808");
                send_cmd_pco(tty_fd, "t2", "33808");
                mksini_update("app_connection", "method", "false");
                sys_set.internet_enabled = false;
                system("systemctl stop QIDILink-client");
            }
            else{
                send_cmd_tsw(tty_fd, "b1", "1");
                send_cmd_tsw(tty_fd, "b3", "1");
                send_cmd_tsw(tty_fd, "b4", "1");
                send_cmd_picc(tty_fd, "b1", "453");
                send_cmd_picc(tty_fd, "b3", "453");
                send_cmd_picc(tty_fd, "b4", "453");
                send_cmd_picc(tty_fd, "b5", "455");
                send_cmd_picc2(tty_fd, "b5", "454");
                send_cmd_pco(tty_fd, "b3", "65535");
                send_cmd_pco(tty_fd, "b4", "65535");
                send_cmd_pco(tty_fd, "t1", "65535");
                send_cmd_pco(tty_fd, "t2", "65535");
                mksini_update("app_connection", "method", "true");
                sys_set.internet_enabled = true;
                system("systemctl restart QIDILink-client");
            }
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_SERVER_SET:
        switch (widget_id)
        {
        case TJC_PAGE_SERVER_SET_MOVE:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_SERVER_SET_FILAMENT:
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_SERVER_SET_AUTO_LEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;
        
        case TJC_PAGE_SERVER_SET_BTN_FILE:
            page_to(TJC_PAGE_FILE_LIST_1);
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();
            get_object_status();
            break;

        case TJC_PAGE_SERVER_SET_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_SERVER_SET_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_SERVER_SET_BACK:
            page_to(TJC_PAGE_NETWORK_SET);
            refresh_lan_model_frpc();
            break;
        
        case TJC_PAGE_SERVER_SET_REFRESH:
            go_to_server_set(0);
            break;

        case TJC_PAGE_SERVER_SET_1:
            page_to(TJC_PAGE_SEARCH_SERVER);
            if (current_server_page * 5 + 1 <= total_server_count) {
                update_server(current_server_page * 5 + 1);
            }
            page_to(TJC_PAGE_SERVER_SET);
            break;

        case TJC_PAGE_SERVER_SET_2:
            page_to(TJC_PAGE_SEARCH_SERVER);
            if (current_server_page * 5 + 2 <= total_server_count) {
                update_server(current_server_page * 5 + 2);
            }
            page_to(TJC_PAGE_SERVER_SET);
            break;

        case TJC_PAGE_SERVER_SET_3:
            page_to(TJC_PAGE_SEARCH_SERVER);
            if (current_server_page * 5 + 3 <= total_server_count) {
                update_server(current_server_page * 5 + 3);
            }
            page_to(TJC_PAGE_SERVER_SET);
            break;

        case TJC_PAGE_SERVER_SET_4:
            page_to(TJC_PAGE_SEARCH_SERVER);
            if (current_server_page * 5 + 4 <= total_server_count) {
                update_server(current_server_page * 5 + 4);
            }
            page_to(TJC_PAGE_SERVER_SET);
            break;

        case TJC_PAGE_SERVER_SET_5:
            page_to(TJC_PAGE_SEARCH_SERVER);
            if (current_server_page * 5 + 5 <= total_server_count) {
                update_server(current_server_page * 5 + 5);
            }
            page_to(TJC_PAGE_SERVER_SET);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_ONLINE_UPDATE:
        switch (widget_id)
        {
        case TJC_PAGE_ONLINE_UPDATE_NEXT:
            btn_update_click_handler();
            // online_update();
            break;
        
        case TJC_PAGE_ONLINE_UPDATE_BACK:
            go_to_about();
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_QIDI_LINK:
        switch (widget_id)
        {
        case TJC_PAGE_QIDI_LINK_MOVE:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_QIDI_LINK_FILAMENT:
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_QIDI_LINK_AUTO_LEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;
        
        case TJC_PAGE_QIDI_LINK_BTN_FILE:
            page_to(TJC_PAGE_FILE_LIST_1);
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();
            get_object_status();
            break;

        case TJC_PAGE_QIDI_LINK_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_QIDI_LINK_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_QIDI_LINK_BACK:
            page_to(TJC_PAGE_NETWORK_SET);
            refresh_lan_model_frpc();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS:
        switch (widget_id)
        {

        case TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS_MOVE:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS_FILAMENT:
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS_AUTO_LEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;
        
        case TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS_FILE:
            page_to(TJC_PAGE_FILE_LIST_1);
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();
            get_object_status();
            break;

        case TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS_BACK:
            page_to(TJC_PAGE_NETWORK_SET);
            refresh_lan_model_frpc();
            break;

        case TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS_OUT:
            // TODO：退出QIDI Link 事件
            page_to(TJC_PAGE_QIDI_LINK_LOG_OUT);
            app_cli.set_bind_status("UNBOUND");
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_QIDI_LINK_LOG_OUT:
        switch (widget_id)
        {
        case TJC_PAGE_QIDI_LINK_LOG_OUT_CANCEL:
            mksini_load();
            tmp_username = mksini_getstring("app", "username", "");
            mksini_free();
            page_to(TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS);
            send_cmd_txt(tty_fd, "t1", tmp_username);
            login_successed_page();
            break;

        case TJC_PAGE_QIDI_LINK_LOG_OUT_CONFIRM:
            page_to(TJC_PAGE_QIDI_LINK);
            send_cmd_vis(tty_fd, "gm0", "1");
            std::cout << "logout" << std::endl;
            user_token = "";
            mksini_load();
            mksini_set("app", "token", user_token);
            mksini_set("app", "username", "");
            mksini_set("app", "avatar", "");
            mksini_save();
            mksini_free();
            system("systemctl stop QIDILink-client");
            break; 

        default:
            break;
        }
        break;

    case TJC_PAGE_QIDI_LINK_LOG_FAIL:
        switch (widget_id)
        {

        case TJC_PAGE_QIDI_LINK_LOG_FAIL_MOVE:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_QIDI_LINK_LOG_FAIL_FILAMENT:
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_QIDI_LINK_LOG_FAIL_AUTO_LEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;
        
        case TJC_PAGE_QIDI_LINK_LOG_FAIL_FILE:
            page_to(TJC_PAGE_FILE_LIST_1);
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();
            get_object_status();
            break;

        case TJC_PAGE_QIDI_LINK_LOG_FAIL_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_QIDI_LINK_LOG_FAIL_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_QIDI_LINK_LOG_FAIL_BACK:
            page_to(TJC_PAGE_QIDI_LINK);
            break;
        
        default:
            break;
        }
        break;
    
    case TJC_PAGE_OFF_UPDATE_CHOOSE:
        switch (widget_id)
        {
        case TJC_PAGE_OFF_UPDATE_CHOOSE_CONFIRM:
            page_to(TJC_PAGE_OFFLINE_UPDATING);
            start_update();
            break;
        
        case TJC_PAGE_OFF_UPDATE_CHOOSE_CANCLE:
            go_to_about();
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_NO_OFFLINE_FILES:
        switch (widget_id)
        {
        case TJC_PAGE_NO_OFFLINE_FILES_CONFIRM:
            go_to_about();
            break;

        default:
            break;
        }
        break;


    default:
        break;
    }
}

void tjc_event_setted_handler(int page_id, int widget_id, unsigned char first, unsigned char second) {
    std::cout << "!!!" << page_id << std::endl;
    std::cout << "!!!" << widget_id << std::endl;
    std::cout << "!!!" << first << std::endl;
    std::cout << "!!!" << second << std::endl;
    int number = (int)((second << 8) + first);
    switch (page_id)
    {
    case TJC_PAGE_PRINTING:
        switch (widget_id)
        {
        case TJC_PAGE_PRINTING_SET_EXTRUDER:
            std::cout << "********************" << number << std::endl;
            if (number > 350) {
                number = 350;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_extruder_target(number);
            send_cmd_val(tty_fd, "n0", std::to_string(number));
            // set_mks_extruder_target(number);
            break;

        case TJC_PAGE_PRINTING_SET_HEATER_BED:
            std::cout << "********************" << number << std::endl;
            if (number > 120) {
                number = 120;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_heater_bed_target(number);
            send_cmd_val(tty_fd, "n1", std::to_string(number));
            // set_mks_heater_bed_target(number);
            break;
        
        case TJC_PAGE_PRINTING_SET_HOT:
            std::cout << "********************" << number << std::endl;
            //2023.4.25-2 修改热腔温度上限
            if (number > 65) {
                number = 65;
            } 
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_hot_target(number);
            send_cmd_val(tty_fd, "n2", std::to_string(number));
            // set_mks_hot_target(number);
            break;

        case TJC_PAGE_PRINTING_SET_FAN_0:
            std::cout << "********************" << number << std::endl;
            if (number > 100) {
                number = 100;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_fan0(number);
            send_cmd_val(tty_fd, "n3", std::to_string(number));
            break;

        case TJC_PAGE_PRINTING_SET_FAN_1:
            std::cout << "********************" << number << std::endl;
            if (number > 100) {
                number = 100;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_fan2(number);
            send_cmd_val(tty_fd, "n4", std::to_string(number));
            break;

        //2.1.2 CLL 新增fan3
        case TJC_PAGE_PRINTING_SET_FAN_3:
            std::cout << "********************" << number << std::endl;
            if (number > 100) {
                number = 100;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_fan3(number);
            send_cmd_val(tty_fd, "n9", std::to_string(number));
            break;

        case TJC_PAGE_PRINTING_SET_SPEED:
            std::cout << "********************" << number << std::endl;
            //2.1.2 CLL 限制喷头移速和挤出上限为150%
            if (number > 150) {
                number = 150;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_printer_speed(number);
            send_cmd_val(tty_fd, "n5", std::to_string(number));
            break;

        case TJC_PAGE_PRINTING_SET_FLOW:
            std::cout << "********************" << number << std::endl;
            //2.1.2 CLL 限制喷头移速和挤出上限为150%
            if (number > 150) {
                number = 150;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_printer_flow(number);
            send_cmd_val(tty_fd, "n6", std::to_string(number));
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_BTN_EXTRUDER:
        // case 0x0b:
            std::cout << "********************" << number << std::endl;
            // filament_keyboard_enabled = false;
            if (number > 350) {
                number = 350;
            }
            set_extruder_target(number);
            //4.2.7 CLL 修改喷头温度保存下限为170
            if (number > 170) {
                set_mks_extruder_target(number);
            }
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_FILAMENT_BTN_HEATER_BED:
        // case 0x0c:
            std::cout << "********************" << number << std::endl;
            // filament_keyboard_enabled = false;
            if (number > 120) {
                number = 120;
            }
            set_heater_bed_target(number);
            set_mks_heater_bed_target(number);
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_FILAMENT_BTN_HOT:
        // case 0x0d:
            std::cout << "********************" << number << std::endl;
            // filament_keyboard_enabled = false;
            //2023.4.25-2 修改热腔温度上限
            if (number > 65) {
                number = 65;
            }
            set_hot_target(number);
            set_mks_hot_target(number);
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_FILAMENT_BTN_FAN_1:
            break;

        case TJC_PAGE_FILAMENT_BTN_FAN_2:
            break;

        case TJC_PAGE_FILAMENT_BTN_FILAMENT_SENSOR:
            // set_filament_sensor();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_SET_TEMP_LEVEL:
        switch (widget_id)
        {
        case TJC_PAGE_SET_TEMP_LEVEL_EXTRUDER:
            std::cout << "********************" << number << std::endl;
            level_mode_printing_extruder_target = number;
            page_to(TJC_PAGE_SET_TEMP_LEVEL);
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_HOT_BED:
            std::cout << "********************" << number << std::endl;
            level_mode_printing_heater_bed_target = number;
            page_to(TJC_PAGE_SET_TEMP_LEVEL);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

void tjc_event_keyboard(char *cmd) {
    MKSLOG("收到的键盘返回的值为 %s\n", cmd);
    MKSLOG("cmd 1  %d\n", cmd[1]);
    MKSLOG("cmd 2  %d\n", cmd[2]);
    char *psk = &cmd[3];
    // int page;
    // int widget;
    // int type;
    // page = cmd[1];
    // widget = cmd[2];
    // type = cmd[3];
    switch (cmd[1]) {
    case TJC_PAGE_WIFI_LIST_2:
        switch (cmd[2])
        {
        case TJC_PAGE_WIFI_LIST_2_SSID_1:
            MKSLOG_RED("收到密码, %d, 密码是%s", strlen(psk), psk);
            printing_wifi_keyboard_enabled = false;
            // page_to(TJC_PAGE_WIFI_LIST_2);
            page_to(TJC_PAGE_WIFI_CONNECT);
            print_ssid_psk(psk);
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_2:
            MKSLOG_RED("收到密码, %d, 密码是%s", strlen(psk), psk);
            printing_wifi_keyboard_enabled = false;
            page_to(TJC_PAGE_WIFI_CONNECT);
            print_ssid_psk(psk);
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_3:
            MKSLOG_RED("收到密码, %d, 密码是%s", strlen(psk), psk);
            
            printing_wifi_keyboard_enabled = false;
            // page_to(TJC_PAGE_WIFI_LIST_2);
            page_to(TJC_PAGE_WIFI_CONNECT);
            print_ssid_psk(psk);
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_4:
            MKSLOG_RED("收到密码, %d, 密码是%s", strlen(psk), psk);
            
            printing_wifi_keyboard_enabled = false;
            // page_to(TJC_PAGE_WIFI_LIST_2);
            page_to(TJC_PAGE_WIFI_CONNECT);
            print_ssid_psk(psk);
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_5:
            MKSLOG_RED("收到密码, %d, 密码是%s", strlen(psk), psk);
            
            printing_wifi_keyboard_enabled = false;
            // page_to(TJC_PAGE_WIFI_LIST_2);
            page_to(TJC_PAGE_WIFI_CONNECT);
            print_ssid_psk(psk);
            break;

        default:
            break;
        }
        break;
    }
}
