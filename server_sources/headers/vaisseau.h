
#ifndef ASTEROID_VAISSEAU_H
#define ASTEROID_VAISSEAU_H

#include <utility>
#include "polyServeur.h"
#include "vec2.h"
#include <boost/log/trivial.hpp>
#include <random>
#include "server_ws.hpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

class vaisseau : public polyServeur {
public:
    vaisseau(std::initializer_list<vec2d> &&liste) : polyServeur(liste), m_life_level(10) { init(); }

    vaisseau(std::string &p_username, std::shared_ptr<WsServer::Connection> p_connection, int type = 1) : m_username(
            p_username), m_connection((p_connection)), m_type(type), m_life_level(10), m_x2_count(30), m_x3_count(15) {
        //BOOST_LOG_TRIVIAL(info)<<"vaisseau() -- username : "<<m_username;
        initialize_poly();
        init();
    }

    void initialize_poly() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(0.0, 100);
        std::vector<vec2d> V;
        vec2d p{LARGEUR / 2 + dist(gen), HAUTEUR / 2 + dist(gen)};
        V.emplace_back(p.x, p.y - 24);
        V.emplace_back(p.x - 7, p.y);
        V.emplace_back(p.x + 7, p.y);
        points = V;

        // center point
        int minX = 0xFFFF, minY = 0xFFFF, maxX = 0, maxY = 0;
        for (const auto &p : points) {
            if (p.x > maxX) maxX = p.x;
            if (p.x < minX) minX = p.x;
            if (p.y > maxY) maxY = p.y;
            if (p.y < minY) minY = p.y;
        }
        this->m_center.x = (minX) + ((maxX - minX) / 2);
        this->m_center.y = (minY) + ((maxY - minY) / 2);
    }

    explicit vaisseau(std::vector<vec2d> points) : polyServeur(std::move(points)) {}

    void rotationDroite(double degree = 2.0) {
        //BOOST_LOG_TRIVIAL(info)<<"rotationDroite() -- username : "<<m_username;
        rotate(degree);
    }

    void rotationGauche(double degree = -2.0) {
        //BOOST_LOG_TRIVIAL(info) << "rotationGauche() -- username : " << m_username;
        rotate(degree);
    }

    void rotate(double deg) {
        for (auto &p : points) {
            p.rotate(deg, this->m_center);
        }
    }

    void avancer(const vec2d &v) {
        //BOOST_LOG_TRIVIAL(info)<<"avancer() -- username : "<<m_username;
        vec2d dir = (this->points[0] - this->m_center).normalize() * 10;
        for (auto &p : points)
            p = p + dir;
        this->m_center = this->m_center + dir;
    }

    void send_message(const std::string &p_message) {
        //BOOST_LOG_TRIVIAL(info)<<"send_message() start";
        if (m_connection == nullptr) {
            //BOOST_LOG_TRIVIAL(warning)<<"trying to send message with NULL m_connection";
            return;
        }
        m_connection->send(p_message);
        //BOOST_LOG_TRIVIAL(info)<<"message sent";

    }

    bool is_me(std::shared_ptr<WsServer::Connection> p_connection) {
        //BOOST_LOG_TRIVIAL(info)<<"is_me() -- p_con : "<<p_connection.get()<<" -- m_con : "<<m_connection.get();
        return p_connection.get() == m_connection.get();
    }

    const std::string &get_username() const {
        return m_username;
    }

    [[nodiscard]] int get_type() const {
        return m_type;
    }

    void set_type(int type) {
        m_type = type;
    }

    void init() {
        m_life_level = 10;
        m_x2_count = 50;
        m_x3_count = 25;
    }

    bool attack(int type = 1) {
        m_life_level -= type;
        bool ret = m_life_level <= 0;
        if (ret) {
            init();
        }
        return ret;
    }

    bool use_x2() {
        if (m_x2_count <= 0)
            return false;
        m_x2_count--;
        return true;
    }

    bool use_x3() {
        if (m_x3_count <= 0)
            return false;
        m_x3_count--;
        return true;
    }

    int get_x3() {
        return m_x3_count;
    }

    int get_x2() {
        return m_x2_count;
    }

    int get_life_lev() const {
        return m_life_level;
    }

private:
    std::string m_username;
    std::shared_ptr<WsServer::Connection> m_connection;

    int m_type;
    int m_life_level;
    int m_x2_count;
    int m_x3_count;
};

#endif //ASTEROID_VAISSEAU_H
