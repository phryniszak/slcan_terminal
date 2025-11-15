/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * slcan_terminal.cpp - Interactive terminal for SLCAN serial communication
 *
 *  Pawel Hryniszak phryniszak@gmail.com
 *		
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <sstream>
#include <dirent.h>
#include <algorithm>

class SlcanTerminal
{
private:
    std::string tty_path;
    int fd;
    std::atomic<bool> running;
    struct termios old_tty_settings;
    struct termios old_stdin_settings;

    void setup_serial_port()
    {
        struct termios tty;

        // Get current settings
        if (tcgetattr(fd, &tty) < 0)
        {
            perror("tcgetattr");
            exit(EXIT_FAILURE);
        }

        // Save old settings
        old_tty_settings = tty;

        // Configure serial port for raw mode
        cfmakeraw(&tty);

        // Set baud rate to 115200 (common for SLCAN devices)
        cfsetospeed(&tty, B115200);
        cfsetispeed(&tty, B115200);

        // 8N1 mode
        tty.c_cflag &= ~PARENB; // No parity
        tty.c_cflag &= ~CSTOPB; // 1 stop bit
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;            // 8 data bits
        tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem control

        // No hardware flow control
        tty.c_cflag &= ~CRTSCTS;

        // No software flow control
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);

        // Raw output
        tty.c_oflag &= ~OPOST;

        // Non-canonical mode, no echo
        tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

        // Read timeout settings
        tty.c_cc[VMIN] = 0;  // Non-blocking
        tty.c_cc[VTIME] = 1; // 0.1 second timeout

        if (tcsetattr(fd, TCSANOW, &tty) < 0)
        {
            perror("tcsetattr");
            exit(EXIT_FAILURE);
        }

        // Flush any existing data
        tcflush(fd, TCIOFLUSH);
    }

    void setup_stdin()
    {
        struct termios tty;

        // Get current stdin settings
        if (tcgetattr(STDIN_FILENO, &tty) < 0)
        {
            perror("tcgetattr stdin");
            exit(EXIT_FAILURE);
        }

        // Save old settings
        old_stdin_settings = tty;

        // Set stdin to raw mode for character-by-character input
        tty.c_lflag &= ~(ICANON | ECHO);
        tty.c_cc[VMIN] = 1;
        tty.c_cc[VTIME] = 0;

        if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) < 0)
        {
            perror("tcsetattr stdin");
            exit(EXIT_FAILURE);
        }
    }

    void restore_stdin()
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_stdin_settings);
    }

    void restore_serial()
    {
        if (fd >= 0)
        {
            tcsetattr(fd, TCSANOW, &old_tty_settings);
        }
    }

    std::string get_feedback_description(const std::string &response)
    {
        // Check if response contains feedback code (starts with #)
        size_t pos = response.find('#');
        if (pos == std::string::npos)
        {
            return "";
        }

        // Extract the character after #
        if (pos + 1 < response.length())
        {
            char code = response[pos + 1];

            switch (code)
            {
            case '\r':
            case '\n':
                return " (Success)";
            case '1':
                return " (Invalid command)";
            case '2':
                return " (Invalid parameter)";
            case '3':
                return " (Adapter must be open)";
            case '4':
                return " (Adapter must be closed)";
            case '5':
                return " (HAL error from ST Microelectronics)";
            case '6':
                return " (Feature not supported/implemented)";
            case '7':
                return " (CAN Tx buffer full - no ACK, 67 packets waiting)";
            case '8':
                return " (CAN bus off - severe error occurred)";
            case '9':
                return " (Sending not possible in silent mode)";
            case ':':
                return " (Baudrate not set)";
            case ';':
                return " (Flash Option Bytes programming failed)";
            case '<':
                return " (Hardware reset required - reconnect USB)";
            default:
                return "";
            }
        }

        return "";
    }

    std::string get_error_description(const std::string &response)
    {
        // Check if response is an error report (format: Exxxxxxxx)
        size_t pos = response.find('E');
        if (pos == std::string::npos)
        {
            return "";
        }

        // Error format: Exxxxxxxx (9 characters total: E + 8 hex digits)
        if (pos + 8 >= response.length())
        {
            return "";
        }

        std::string error_code = response.substr(pos + 1, 8);

        // Validate it's all hex digits
        for (char c : error_code)
        {
            if (!std::isxdigit(c))
            {
                return "";
            }
        }

        std::string desc = " (";

        // Digit 1: Bus Status
        char bus_status = error_code[0];
        switch (bus_status)
        {
        case '0':
            desc += "Bus Active";
            break;
        case '1':
            desc += "Warning Level";
            break;
        case '2':
            desc += "Bus Passive";
            break;
        case '3':
            desc += "Bus Off";
            break;
        default:
            desc += "Unknown Bus Status";
        }

        // Digit 2: Last Protocol Error
        char protocol_error = error_code[1];
        if (protocol_error != '0')
        {
            desc += ", ";
            switch (protocol_error)
            {
            case '1':
                desc += "Bit stuffing error";
                break;
            case '2':
                desc += "Frame format error";
                break;
            case '3':
                desc += "No ACK received";
                break;
            case '4':
                desc += "Recessive bit error";
                break;
            case '5':
                desc += "Dominant bit error";
                break;
            case '6':
                desc += "CRC error";
                break;
            default:
                desc += "Unknown protocol error";
            }
        }

        // Digits 3+4: Firmware Error Flags (hex)
        std::string fw_flags = error_code.substr(2, 2);
        int flags = std::stoi(fw_flags, nullptr, 16);

        if (flags != 0)
        {
            desc += ", ";
            bool first = true;
            if (flags & 0x01)
            {
                desc += "Rx Failed";
                first = false;
            }
            if (flags & 0x02)
            {
                if (!first)
                    desc += "+";
                desc += "Tx Failed";
                first = false;
            }
            if (flags & 0x04)
            {
                if (!first)
                    desc += "+";
                desc += "CAN Tx buffer overflow";
                first = false;
            }
            if (flags & 0x08)
            {
                if (!first)
                    desc += "+";
                desc += "USB IN buffer overflow";
                first = false;
            }
            if (flags & 0x10)
            {
                if (!first)
                    desc += "+";
                desc += "Tx Timeout";
                first = false;
            }
        }

        // Digits 5+6: Tx Error Count
        std::string tx_err = error_code.substr(4, 2);
        int tx_count = std::stoi(tx_err, nullptr, 16);

        // Digits 7+8: Rx Error Count
        std::string rx_err = error_code.substr(6, 2);
        int rx_count = std::stoi(rx_err, nullptr, 16);

        desc += ", Tx Errors: " + std::to_string(tx_count);
        desc += ", Rx Errors: " + std::to_string(rx_count);
        desc += ")";

        return desc;
    }

    void receive_thread_func()
    {
        char buf[256];

        while (running)
        {
            int n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0)
            {
                buf[n] = '\0';
                std::string response(buf);

                // Split by \r in case multiple messages are received
                std::vector<std::string> messages;
                std::stringstream ss(response);
                std::string message;

                while (std::getline(ss, message, '\r'))
                {
                    if (!message.empty())
                    {
                        messages.push_back(message);
                    }
                }

                // Process each message separately
                for (const auto &msg : messages)
                {
                    // Try both feedback and error descriptions
                    std::string description = get_feedback_description(msg);
                    if (description.empty())
                    {
                        description = get_error_description(msg);
                    }

                    // Remove trailing newline for cleaner display
                    std::string display_response = msg;
                    while (!display_response.empty() && display_response.back() == '\n')
                    {
                        display_response.pop_back();
                    }

                    std::cout << "\r\033[K[RX] " << display_response;
                    if (!description.empty())
                    {
                        std::cout << description;
                    }
                    std::cout << std::endl;
                }
                std::cout << "> " << std::flush;
            }
            usleep(10000); // 10ms delay to prevent busy-waiting
        }
    }

public:
    SlcanTerminal(const std::string &tty) : tty_path(tty), fd(-1), running(false) {}

    ~SlcanTerminal()
    {
        if (running)
        {
            stop();
        }
        if (fd >= 0)
        {
            restore_serial();
            close(fd);
        }
    }

    bool open_device()
    {
        // Check if the path exists and resolve symlinks
        struct stat st;
        if (stat(tty_path.c_str(), &st) < 0)
        {
            perror(tty_path.c_str());
            return false;
        }

        // Check if it's a character device
        if (!S_ISCHR(st.st_mode))
        {
            std::cerr << "Error: " << tty_path << " is not a character device" << std::endl;
            return false;
        }

        // Open the device
        fd = open(tty_path.c_str(), O_RDWR | O_NOCTTY);
        if (fd < 0)
        {
            perror(tty_path.c_str());
            return false;
        }

        // Try to get exclusive access to the serial port
        // This will fail if the port is already in use
        if (ioctl(fd, TIOCEXCL) < 0)
        {
            perror("Cannot get exclusive access (port already in use?)");
            close(fd);
            fd = -1;
            return false;
        }

        // Try to configure as serial port - this will fail if it's not a TTY
        struct termios tty;
        if (tcgetattr(fd, &tty) < 0)
        {
            std::cerr << "Error: " << tty_path << " - cannot get terminal attributes (not a TTY?)" << std::endl;
            close(fd);
            fd = -1;
            return false;
        }

        setup_serial_port();
        return true;
    }

    char encode_dlc(int byte_count)
    {
        // Convert byte count to SLCAN DLC format
        if (byte_count <= 8)
        {
            return '0' + byte_count;
        }
        else
        {
            // CAN FD DLC encoding
            if (byte_count <= 12)
                return '9';
            else if (byte_count <= 16)
                return 'A';
            else if (byte_count <= 20)
                return 'B';
            else if (byte_count <= 24)
                return 'C';
            else if (byte_count <= 32)
                return 'D';
            else if (byte_count <= 48)
                return 'E';
            else
                return 'F';
        }
    }

    std::string convert_cansend_format(const std::string &input)
    {
        // Check if input contains # (cansend format: <packet_type><can_id>#<data>)
        size_t hash_pos = input.find('#');
        if (hash_pos == std::string::npos)
        {
            // No # found, return as-is (already in SLCAN format)
            return input;
        }

        // Extract packet type (first character)
        if (input.empty())
        {
            return input;
        }

        char packet_type = input[0];

        // Validate packet type (t, T, r, R, d, D, b, B)
        if (packet_type != 't' && packet_type != 'T' &&
            packet_type != 'r' && packet_type != 'R' &&
            packet_type != 'd' && packet_type != 'D' &&
            packet_type != 'b' && packet_type != 'B')
        {
            std::cerr << "Error: Invalid packet type '" << packet_type << "' (use t,T,r,R,d,D,b,B)" << std::endl;
            return input;
        }

        // Split into CAN ID and data parts (skip packet type)
        std::string can_id_str = input.substr(1, hash_pos - 1);
        std::string data_str = input.substr(hash_pos + 1);

        // Remove dots and spaces from data
        std::string clean_data;
        for (char c : data_str)
        {
            if (c != '.' && c != ' ')
            {
                clean_data += c;
            }
        }

        // Validate data is hex
        for (char c : clean_data)
        {
            if (!std::isxdigit(c))
            {
                std::cerr << "Error: Invalid hex data: " << clean_data << std::endl;
                return input; // Return original on error
            }
        }

        // Calculate DLC (data length in bytes)
        if (clean_data.length() % 2 != 0)
        {
            std::cerr << "Error: Data must have even number of hex digits" << std::endl;
            return input;
        }

        int dlc = clean_data.length() / 2;
        if (dlc > 64)
        {
            std::cerr << "Error: Data too long (max 64 bytes)" << std::endl;
            return input;
        }

        // Convert DLC to SLCAN format
        char dlc_char = encode_dlc(dlc);

        // Determine ID format based on packet type
        std::string formatted_id;
        bool is_extended = (packet_type == 'T' || packet_type == 'R' ||
                            packet_type == 'D' || packet_type == 'B');

        if (is_extended)
        {
            // Extended 29-bit ID (8 hex digits)
            if (can_id_str.length() > 8)
            {
                std::cerr << "Error: Extended CAN ID too long (max 8 hex digits)" << std::endl;
                return input;
            }
            formatted_id = std::string(8 - can_id_str.length(), '0') + can_id_str;
        }
        else
        {
            // Standard 11-bit ID (3 hex digits)
            if (can_id_str.length() > 3)
            {
                std::cerr << "Error: Standard CAN ID too long (max 3 hex digits)" << std::endl;
                return input;
            }
            formatted_id = std::string(3 - can_id_str.length(), '0') + can_id_str;
        }

        // Validate CAN ID is hex
        for (char c : can_id_str)
        {
            if (!std::isxdigit(c))
            {
                std::cerr << "Error: Invalid CAN ID (must be hex): " << can_id_str << std::endl;
                return input;
            }
        }

        // Construct SLCAN packet
        std::string slcan_packet;
        slcan_packet += packet_type;
        slcan_packet += formatted_id;
        slcan_packet += dlc_char;
        slcan_packet += clean_data;

        return slcan_packet;
    }

    void send_command(const std::string &cmd, bool show_output = true)
    {
        // Convert cansend format to SLCAN if needed
        std::string command = convert_cansend_format(cmd);

        // Add carriage return if not present
        if (command.empty() || command.back() != '\r')
        {
            command += '\r';
        }

        ssize_t written = write(fd, command.c_str(), command.length());
        if (written < 0)
        {
            perror("write");
        }
        else if (show_output)
        {
            std::cout << "[TX] " << command;
            if (command.back() == '\r')
            {
                std::cout << std::endl;
            }
        }
    }

    void send_init_commands(const std::vector<std::string> &commands)
    {
        if (commands.empty())
        {
            return;
        }

        std::cout << "\n=== Sending initialization commands ===" << std::endl;

        for (const auto &cmd : commands)
        {
            std::cout << "[INIT] " << cmd << std::endl;
            send_command(cmd, false);
            usleep(50000); // 50ms delay between commands

            // Try to read response
            char buf[256];
            usleep(50000); // Wait for response
            int n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0)
            {
                buf[n] = '\0';
                std::string response(buf);

                // Split by \r in case multiple messages are received
                std::vector<std::string> messages;
                std::stringstream ss(response);
                std::string message;

                while (std::getline(ss, message, '\r'))
                {
                    if (!message.empty())
                    {
                        messages.push_back(message);
                    }
                }

                // Process each message separately
                for (const auto &msg : messages)
                {
                    // Try both feedback and error descriptions
                    std::string description = get_feedback_description(msg);
                    if (description.empty())
                    {
                        description = get_error_description(msg);
                    }

                    // Remove trailing newline for cleaner display
                    std::string display_response = msg;
                    while (!display_response.empty() && display_response.back() == '\n')
                    {
                        display_response.pop_back();
                    }

                    std::cout << "[RESP] " << display_response;
                    if (!description.empty())
                    {
                        std::cout << description;
                    }
                    std::cout << std::endl;
                }
            }
        }

        std::cout << "=== Initialization complete ===\n"
                  << std::endl;
    }

    void run_terminal()
    {
        running = true;

        // Start receiver thread
        std::thread rx_thread(&SlcanTerminal::receive_thread_func, this);

        setup_stdin();

        std::cout << "\n=== SLCAN Terminal ===" << std::endl;
        std::cout << "Connected to: " << tty_path << std::endl;
        std::cout << "Commands: Enter SLCAN commands (e.g., 'V' for version, 'O' to open)" << std::endl;
        std::cout << "Special: 'quit' or 'exit' to close, Ctrl+C to abort" << std::endl;
        std::cout << "======================\n"
                  << std::endl;

        std::string input_buffer;

        while (running)
        {
            std::cout << "> " << std::flush;

            // Read line from stdin
            input_buffer.clear();
            char ch;
            while (read(STDIN_FILENO, &ch, 1) > 0)
            {
                if (ch == '\n' || ch == '\r')
                {
                    std::cout << std::endl;
                    break;
                }
                else if (ch == 127 || ch == 8)
                { // Backspace
                    if (!input_buffer.empty())
                    {
                        input_buffer.pop_back();
                        std::cout << "\b \b" << std::flush;
                    }
                }
                else if (ch == 3)
                { // Ctrl+C
                    running = false;
                    break;
                }
                else if (ch >= 32 && ch < 127)
                { // Printable characters
                    input_buffer += ch;
                    std::cout << ch << std::flush;
                }
            }

            if (!running)
                break;

            // Process command
            if (!input_buffer.empty())
            {
                if (input_buffer == "quit" || input_buffer == "exit")
                {
                    running = false;
                    break;
                }

                send_command(input_buffer);
            }
        }

        restore_stdin();

        // Wait for receiver thread to finish
        rx_thread.join();

        std::cout << "\nTerminal closed." << std::endl;
    }

    void stop()
    {
        running = false;
    }
};

void print_usage(const char *prg)
{
    std::cerr << prg << " - Interactive terminal for SLCAN serial communication\n"
              << std::endl;
    std::cerr << "Usage: " << prg << " [options] [tty_device]\n"
              << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -h, --help         Show this help message" << std::endl;
    std::cerr << "  -i, --init <cmds>  Initialization commands (comma-separated)" << std::endl;
    std::cerr << "                     Use double quotes to protect commas within commands" << std::endl;
    std::cerr << "\nIf no tty_device is specified, the tool will automatically search" << std::endl;
    std::cerr << "for the first device in /dev containing 'slcan' in its name.\n"
              << std::endl;
    std::cerr << "\nExamples:" << std::endl;
    std::cerr << "  " << prg << "                         (auto-detect SLCAN device)" << std::endl;
    std::cerr << "  " << prg << " /dev/ttyUSB0            (specify device)" << std::endl;
    std::cerr << "  " << prg << " -i \"C,S6,O\"            (auto-detect + init commands)" << std::endl;
    std::cerr << "  " << prg << " -i \"C,S6,O\" /dev/ttyUSB0" << std::endl;
    std::cerr << "  " << prg << " --init \"C,V,S6,ON\" /dev/ttyS1" << std::endl;
    std::cerr << "  " << prg << " -i 's\"1,119,40,40\"'    (custom bitrate with quoted commas)" << std::endl;
    std::cerr << "  " << prg << " -i 'C,s\"1,119,40,40\",ON' (multiple commands with quotes)" << std::endl;
    std::cerr << "\nCommon SLCAN commands:" << std::endl;
    std::cerr << "  V       - Get version and serial number" << std::endl;
    std::cerr << "  S0-S8   - Set CAN speed (0=10k, 4=125k, 6=500k, 8=1000k)" << std::endl;
    std::cerr << "  O       - Open channel (normal mode)" << std::endl;
    std::cerr << "  ON      - Open channel (normal mode, SLCAN 2.5)" << std::endl;
    std::cerr << "  OS      - Open channel (silent mode)" << std::endl;
    std::cerr << "  L       - Open channel (listen-only mode)" << std::endl;
    std::cerr << "  C       - Close channel" << std::endl;
    std::cerr << "  F       - Read status flags" << std::endl;
    std::cerr << "\nSending CAN frames (simplified syntax with #):" << std::endl;
    std::cerr << "  <type><can_id>#<data>  - Auto-calculates DLC, supports dots" << std::endl;
    std::cerr << "  Packet types: t/T (classic), r/R (RTR), d/D (FD), b/B (FD+BRS)" << std::endl;
    std::cerr << "  Examples:" << std::endl;
    std::cerr << "    t123#DEADBEEF       -> t12304DEADBEEF" << std::endl;
    std::cerr << "    t7E0#11.22.33.44    -> t7E00411223344" << std::endl;
    std::cerr << "    T18AABBCC#112233    -> T18AABBCC03112233" << std::endl;
    std::cerr << "    r123#               -> r1230 (RTR with DLC=0)" << std::endl;
    std::cerr << "\nRaw SLCAN format:" << std::endl;
    std::cerr << "  tiiildd          - Transmit standard CAN frame" << std::endl;
    std::cerr << "  Tiiiiiiiildd     - Transmit extended CAN frame" << std::endl;
    std::cerr << "  riiil            - Transmit standard RTR frame" << std::endl;
    std::cerr << "  Riiiiiiiil       - Transmit extended RTR frame" << std::endl;
    std::cerr << std::endl;
}

std::vector<std::string> parse_commands(const std::string &cmd_string)
{
    std::cout << "command string: " << cmd_string << std::endl;

    std::vector<std::string> commands;
    std::string current_cmd;
    bool in_quotes = false;

    for (size_t i = 0; i < cmd_string.length(); i++)
    {
        char c = cmd_string[i];

        if (c == '"')
        {
            // Toggle quote mode but don't add the quote character
            in_quotes = !in_quotes;
        }
        else if (c == ',' && !in_quotes)
        {
            // Comma outside quotes - this is a separator
            // Trim whitespace from current command
            size_t start = current_cmd.find_first_not_of(" \t");
            size_t end = current_cmd.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos)
            {
                commands.push_back(current_cmd.substr(start, end - start + 1));
            }
            current_cmd.clear();
        }
        else
        {
            // Regular character or comma inside quotes
            current_cmd += c;
        }
    }

    // Don't forget the last command
    size_t start = current_cmd.find_first_not_of(" \t");
    size_t end = current_cmd.find_last_not_of(" \t");
    if (start != std::string::npos && end != std::string::npos)
    {
        commands.push_back(current_cmd.substr(start, end - start + 1));
    }

    return commands;
}

std::string find_slcan_device()
{
    std::vector<std::string> candidates;

    // Search /dev/serial/by-id for USB devices with "slcan" in the name
    DIR *dir = opendir("/dev/serial/by-id");
    if (dir)
    {
        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            std::string name(entry->d_name);

            // Skip . and ..
            if (name == "." || name == "..")
            {
                continue;
            }

            // Convert to lowercase for case-insensitive search
            std::string name_lower = name;
            std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);

            // Check if the symlink name contains "slcan"
            if (name_lower.find("slcan") != std::string::npos)
            {
                // Read the symlink to get the actual device path
                std::string symlink_path = "/dev/serial/by-id/" + name;
                char target[PATH_MAX];
                ssize_t len = readlink(symlink_path.c_str(), target, sizeof(target) - 1);

                if (len > 0)
                {
                    target[len] = '\0';
                    std::string target_path(target);

                    // Convert relative path (../../ttyACM0) to absolute path (/dev/ttyACM0)
                    // Extract the device name (e.g., ttyACM0)
                    size_t last_slash = target_path.find_last_of('/');
                    if (last_slash != std::string::npos)
                    {
                        std::string dev_name = target_path.substr(last_slash + 1);
                        std::string dev_path = "/dev/" + dev_name;
                        candidates.push_back(dev_path);
                    }
                }
            }
        }
        closedir(dir);
    }

    // Sort to get consistent ordering (prefer lower numbers)
    std::sort(candidates.begin(), candidates.end());

    if (!candidates.empty())
    {
        return candidates[0];
    }

    return "";
}

int main(int argc, char **argv)
{
    int opt;
    std::vector<std::string> init_commands;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"init", required_argument, 0, 'i'},
        {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "hi:", long_options, nullptr)) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_usage(argv[0]);
            return 0;
        case 'i':
            init_commands = parse_commands(optarg);
            break;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    std::string tty;

    if (optind >= argc)
    {
        // No TTY specified, try to find one
        std::cout << "No TTY device specified, searching for SLCAN device..." << std::endl;
        tty = find_slcan_device();

        if (tty.empty())
        {
            std::cerr << "Error: No SLCAN device found in /dev\n"
                      << std::endl;
            std::cerr << "Please specify a TTY device manually.\n"
                      << std::endl;
            print_usage(argv[0]);
            return 1;
        }

        std::cout << "Found SLCAN device: " << tty << std::endl;
    }
    else
    {
        tty = argv[optind];
    }

    SlcanTerminal terminal(tty);

    if (!terminal.open_device())
    {
        std::cerr << "Failed to open device: " << tty << std::endl;
        return 1;
    }

    // Send initialization commands if provided
    if (!init_commands.empty())
    {
        terminal.send_init_commands(init_commands);
    }

    terminal.run_terminal();

    return 0;
}
