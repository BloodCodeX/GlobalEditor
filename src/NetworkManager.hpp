#pragma once
#define ASIO_STANDALONE
#define _WIN32_WINNT 0x0A00

#include <asio.hpp>
#include <Geode/Geode.hpp>
#include <queue>
#include <mutex>
#include <thread>
#include <string>
#include <vector>
#include <sstream>

using namespace geode::prelude;
using asio::ip::tcp;

class NetworkManager {
private:
    asio::io_context m_context;
    std::unique_ptr<tcp::socket> m_socket;
    std::queue<std::string> m_msgQueue;
    std::mutex m_mutex;
    bool m_isRunning = false;

    NetworkManager() {}

public:
    static NetworkManager& get() {
        static NetworkManager instance;
        return instance;
    }

    void start(std::string ip, int port) {
        if (m_isRunning) return;
        m_isRunning = true;

        std::thread([this, ip, port]() {
            try {
                m_socket = std::make_unique<tcp::socket>(m_context);
                tcp::resolver resolver(m_context);
                asio::connect(*m_socket, resolver.resolve(ip, std::to_string(port)));
                log::info("Connected to server!");

                // Поток для ЧТЕНИЯ данных от сервера
                std::thread([this]() {
                    try {
                        asio::streambuf buffer;
                        while (m_isRunning) {
                            asio::read_until(*m_socket, buffer, '\n');
                            std::string msg;
                            std::istream is(&buffer);
                            std::getline(is, msg);

                            if (!msg.empty()) {
                                // Передаем данные в игру через планировщик Geode
                                Loader::get()->queueInMainThread([msg]() {
                                    handleServerMessage(msg);
                                });
                            }
                        }
                    } catch (...) { m_isRunning = false; }
                }).detach();

                // Цикл ОТПРАВКИ данных
                while (m_isRunning) {
                    std::string msg;
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        if (!m_msgQueue.empty()) {
                            msg = m_msgQueue.front() + "\n";
                            m_msgQueue.pop();
                        }
                    }
                    if (!msg.empty()) {
                        asio::write(*m_socket, asio::buffer(msg));
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
            } catch (std::exception& e) {
                log::error("Network error: {}", e.what());
                m_isRunning = false;
            }
        }).detach();
    }

    void send(std::string data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_msgQueue.push(data);
    }

    // Парсим строку "ID;X;Y" и ставим блок
    static void handleServerMessage(std::string msg) {
        std::vector<std::string> tokens;
        std::stringstream ss(msg);
        std::string item;
        while (std::getline(ss, item, ';')) tokens.push_back(item);

        if (tokens.size() >= 3) {
            int id = std::stoi(tokens[0]);
            float x = std::stof(tokens[1]);
            float y = std::stof(tokens[2]);

            auto editor = LevelEditorLayer::get();
            if (editor) {
                // Создаем блок от другого игрока
                auto obj = editor->m_editorUI->createObject(id, {x, y});
            }
        }
    }
};
