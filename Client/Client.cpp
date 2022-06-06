#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <fstream>
#include <sstream>
using boost::asio::ip::tcp;

constexpr char SERVER_IP[] = "127.0.0.1";
constexpr char SERVER_PORT[] = "8000";

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Provide path to file: " << argv[0] << " <file path>" << std::endl;
        return __LINE__;
    }
    try {
        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        boost::system::error_code error;
        tcp::socket socket(io_service);
        boost::asio::connect(socket, resolver.resolve({ SERVER_IP, SERVER_PORT }));
        std::cout << "Connected to " << SERVER_IP << ":" << SERVER_PORT << std::endl;

        std::ifstream source_file(argv[1], std::ios_base::binary | std::ios_base::ate);
        if (!source_file) {
            std::cout << "Failed to open " << argv[1] << std::endl;
            return __LINE__;
        }

        size_t file_size = source_file.tellg();
        source_file.seekg(0);
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << argv[1] << "\n" << file_size << "\n\n";
        boost::asio::write(socket, request);

        std::cout << "Start sending file content.\n";
        boost::array<char, 1024> buf;
        while(true) {
            if (source_file.eof() == false) {
                source_file.read(buf.c_array(), (std::streamsize)buf.size());
                if (source_file.gcount() <= 0) {
                    std::cout << "Read file error " << std::endl;
                    return __LINE__;
                }

                boost::asio::write(socket, boost::asio::buffer(buf.c_array(),
                                   source_file.gcount()), boost::asio::transfer_all(), error);
                if (error) {
                    std::cout << "Send error:" << error << std::endl;
                    return __LINE__;
                }
            }
            else
                break;
        }
        std::cout << "File \"" << argv[1] << "\" sending was completed successfully.\n";
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}