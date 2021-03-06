
#ifndef ASTEROID_GAME_H
#define ASTEROID_GAME_H

#include <iostream>
#include <utility>
#include "asteroid.h"
#include "laser.h"
#include "vaisseau.h"
#include "grandAsteroid.h"
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>

namespace pt = boost::property_tree;

class game {
public:

    explicit game(std::string &game_id) : m_game_id{game_id},
                                          asteroids{}, lasers{}, vaisseaux{}, m_score1(0), vaisseaux2{}, m_score2(0) {
        m_lock = std::make_shared<std::mutex>();
        cc = 0;
        end = false;
        m_not = false;
    }

    void start() {
        BOOST_LOG_TRIVIAL(info) << "start() game_id : " << m_game_id;
        m_lock->lock();
        end = false;
        for (int i = 0; i < 10; ++i)
            asteroids.emplace_back(new grandAsteroid{});
        m_lock->unlock();
    }

    void run() {
        m_lock->lock();
        for (auto &i : asteroids)
            i->step();
        for (unsigned long i = 0; i < lasers.size(); ++i) {
            if (lasers[i].isBordure()) lasers.erase(lasers.begin() + i);
            else lasers[i].step();
        }
        for (auto &i : vaisseaux) // pour faire passer les vaiseaux de l'autre coter
            i->step();

        for (auto &i : vaisseaux2) // pour faire passer les vaiseaux de l'autre coter
            i->step();

        collision_lasers_asteroids();
        collision_vaisseaux_asteroids();
        collision_lasers_vaisseaux();
        if(asteroids.empty()){
            end = true;
        }
        m_lock->unlock();
    }

    void collision_lasers_vaisseaux() {
        for (unsigned long i = 0; i < lasers.size(); ++i) {
            unsigned long j;
            for (j = 0; j < vaisseaux.size() && !lasers[i].intersecte(*vaisseaux[j]); ++j) {}
            if (j < vaisseaux.size()) { // vaisseau j touche par laser i
                if (vaisseaux[j]->get_type() != lasers[j].get_type()) { // ennemi
                    if(lasers[j].get_type() == 1){
                        m_score1 +=200;
                    }
                    if (vaisseaux[j]->attack()) {
                        vaisseaux[j]->initialize_poly();
                    }
                }
            }
            for (j = 0; j < vaisseaux2.size() && !lasers[i].intersecte(*vaisseaux2[j]); ++j) {}
            if (j < vaisseaux2.size()) { // vaisseau2 j touche par laser i
                if (vaisseaux2[j]->get_type() != lasers[j].get_type()) { // ennemi
                    if(lasers[j].get_type() == 2){
                        m_score2 +=200;
                    }

                    if (vaisseaux2[j]->attack()) {
                        vaisseaux2[j]->initialize_poly();
                    }
                }
            }
        }
    }

    void on_game_end(){
        if(m_not)
            return;
        m_not = true;
        pt::ptree win,lost;
        win.put("type","end_of_game");
        lost.put("type","end_of_game");
        win.put("won","true");
        lost.put("won","false");

        win.put("s1",std::to_string(m_score1));
        lost.put("s1",std::to_string(m_score2));
        win.put("s2",std::to_string(m_score2));
        lost.put("s2",std::to_string(m_score1));

        if(m_score1 > m_score2){
            if(vaisseaux2.empty()){
                win.put("mode","coop");
            }
            else{
                win.put("mode","combat");
                lost.put("mode","combat");
            }
            std::stringstream w;
            std::stringstream l;

            boost::property_tree::json_parser::write_json(w, win);
            boost::property_tree::json_parser::write_json(l, lost);

            std::string win_m = w.str();
            std::string lost_m = l.str();

            for (auto& p : vaisseaux){
                p->send_message(win_m);
            }

            for (auto& p : vaisseaux2){
                p->send_message(lost_m);

            }
        }
        else{
            if(vaisseaux.empty()){
                win.put("mode","coop");
            }
            else{
                win.put("mode","combat");
                lost.put("mode","combat");
            }
            std::stringstream w;
            std::stringstream l;

            boost::property_tree::json_parser::write_json(w, win);
            boost::property_tree::json_parser::write_json(l, lost);

            std::string win_m = w.str();
            std::string lost_m = l.str();

            for (auto& p : vaisseaux){
                p->send_message(lost_m);
            }

            for (auto& p : vaisseaux2){
                p->send_message(win_m);

            }

        }

    }

    void collision_lasers_asteroids() {
        for (unsigned long i = 0; i < asteroids.size(); ++i) {
            unsigned long j;
            for (j = 0; j < lasers.size() && !asteroids[i]->intersecte(lasers[j]); ++j) {}
            // si le laser j intersecte avec l'asteroid i :
            if (j < lasers.size()) {
                if(lasers[j].get_type() == 1){
                    this->m_score1 +=100;
                }
                else{
                    this->m_score2 += 100;
                }
                auto rocks = asteroids[i]->generationDestruction();
                for (const auto &p : rocks)
                    asteroids.emplace_back(new petitAsteroid(p));
                asteroids.erase(asteroids.begin() + i);
            }
        }
    }

    void collision_vaisseaux_asteroids() {
        for (unsigned long i = 0; i < asteroids.size(); ++i) {
            bool b = false;
            unsigned long j;
            for (j = 0; j < vaisseaux.size() && !asteroids[i]->intersecte(*vaisseaux[j]); ++j) {}
            // si le vaisseau j intersecte avec l'asteroid i :
            if (j < vaisseaux.size()) {
                if(m_score1 > 0){
                    m_score1 -=100;
                }
                vaisseaux[j]->initialize_poly();
                b = true;
            }
            for (j = 0; j < vaisseaux2.size() && !asteroids[i]->intersecte(*vaisseaux2[j]); ++j) {}
            // si le vaisseau2 j intersecte avec l'asteroid i :
            if (j < vaisseaux2.size()) {
                if(m_score2 > 0){
                    m_score2-=100;
                }
                vaisseaux2[j]->initialize_poly();
                b = true;
            }
            if (b) asteroids.erase(asteroids.begin() + i);
        }
    }

    void add_new_player(std::string &p_username, std::shared_ptr<WsServer::Connection> p_connection) {
        std::shared_ptr<vaisseau> tmp = std::make_shared<vaisseau>(vaisseau(p_username, p_connection, 1));
        m_lock->lock();
        vaisseaux.push_back(tmp);
        m_lock->unlock();
        start();
    }

    std::shared_ptr<vaisseau> has_player(std::shared_ptr<WsServer::Connection> p_connection) {
        m_lock->lock();
        for (const auto &v : vaisseaux)
            if (v->is_me(p_connection)) {
                m_lock->unlock();
                return v;
            }
        m_lock->unlock();
        return nullptr;
    }

    void add_new_player2(std::string &p_username, std::shared_ptr<WsServer::Connection> p_connection) {
        std::shared_ptr<vaisseau> tmp = std::make_shared<vaisseau>(vaisseau(p_username, p_connection, 2));
        m_lock->lock();
        vaisseaux2.push_back(tmp);
        m_lock->unlock();
        start();
    }

    std::shared_ptr<vaisseau> has_player2(std::shared_ptr<WsServer::Connection> p_connection) {
        m_lock->lock();
        for (const auto &v : vaisseaux2)
            if (v->is_me(p_connection)) {
                m_lock->unlock();
                return v;
            }
        m_lock->unlock();
        return nullptr;
    }

    void move_forward(std::shared_ptr<vaisseau> player) {
        BOOST_LOG_TRIVIAL(info) << "move_forward() -- start  username : " << player->get_username();
        player->avancer({});
        BOOST_LOG_TRIVIAL(info) << "move_forward() -- end --  username : " << player->get_username();
    }

    void move_backward(std::shared_ptr<vaisseau> player) {
        BOOST_LOG_TRIVIAL(info) << "move_backward() -- start -- username : " << player->get_username();
        player->avancer({});
        BOOST_LOG_TRIVIAL(info) << "move_backward() -- end -- username : " << player->get_username();
    }

    void rotate_left(std::shared_ptr<vaisseau> player) {
        BOOST_LOG_TRIVIAL(info) << "rotate_left() -- start -- username : " << player->get_username();
        player->rotationGauche(-6.0);
        BOOST_LOG_TRIVIAL(info) << "move_backward() -- end -- username : " << player->get_username();

    }

    void rotate_right(std::shared_ptr<vaisseau> player) {
        BOOST_LOG_TRIVIAL(info) << "rotate_right() --start -- username : " << player->get_username();
        player->rotationDroite(6.0);
        BOOST_LOG_TRIVIAL(info) << "move_backward() -- end -- username : " << player->get_username();
    }

    void fire(std::shared_ptr<vaisseau> player, const std::string &type = "X1") {
        BOOST_LOG_TRIVIAL(info) << "fire() -- start -- username : " << player->get_username();

        vec2d from = player->points[0];
        vec2d to = (player->points[0] - player->m_center).normalize() * 10 + from;
        m_lock->lock();
        lasers.emplace_back(to, from, player->get_type());
        if (type == "X2" && player->use_x2()) {
            lasers.emplace_back(vec2d::rotate_s(-10, to, from), from, player->get_type());
            lasers.emplace_back(vec2d::rotate_s(10, to, from), from, player->get_type());
            lasers.emplace_back(vec2d::rotate_s(20, to, from), from, player->get_type());
            lasers.emplace_back(vec2d::rotate_s(-20, to, from), from, player->get_type());

        } else if (type == "X3" && player->use_x3()) {

            lasers.emplace_back(vec2d::rotate_s(10.0, to, from), from, player->get_type());
            lasers.emplace_back(vec2d::rotate_s(20.0, to, from), from, player->get_type());
            lasers.emplace_back(vec2d::rotate_s(30.0, to, from), from, player->get_type());
            lasers.emplace_back(vec2d::rotate_s(40.0, to, from), from, player->get_type());


            lasers.emplace_back(vec2d::rotate_s(-10, to, from), from, player->get_type());
            lasers.emplace_back(vec2d::rotate_s(-20, to, from), from, player->get_type());
            lasers.emplace_back(vec2d::rotate_s(-30, to, from), from, player->get_type());
            lasers.emplace_back(vec2d::rotate_s(-40.0, to, from), from, player->get_type());
        }

        m_lock->unlock();
        BOOST_LOG_TRIVIAL(info) << "NULL LASER" << player->get_username();
        BOOST_LOG_TRIVIAL(info) << "fire() -- end -- username : " << player->get_username();
    }

    void broadcast_view() {
        if(end){
            BOOST_LOG_TRIVIAL(info) << "broadcast_view() GAME ENDED AND NOT DELETED================\n";//<<view;
            return;
        }
        std::string view = get_game_view();
        BOOST_LOG_TRIVIAL(info) << "broadcast_view() -- count : "<<asteroids.size()<<" \n";//<<view;
        for (const auto &p : vaisseaux)
            p->send_message(view);

        for (const auto &p : vaisseaux2)
            p->send_message(view);


        if (cc == 0) {
            cc = 30;
            pt::ptree root = get_infos();
            std::string infos;
            for (const auto &p : vaisseaux) {
                pt::ptree tmp = root;
                tmp.put("X2", boost::lexical_cast<std::string>(p->get_x2()));
                tmp.put("X3", boost::lexical_cast<std::string>(p->get_x3()));
                std::stringstream sz;
                boost::property_tree::json_parser::write_json(sz, tmp);
                p->send_message(sz.str());

            }

            for (const auto &p : vaisseaux2) {
                pt::ptree tmp = root;
                tmp.put("X2", boost::lexical_cast<std::string>(p->get_x2()));
                tmp.put("X3", boost::lexical_cast<std::string>(p->get_x3()));
                std::stringstream sz;
                boost::property_tree::json_parser::write_json(sz, tmp);
                p->send_message(sz.str());
            }
        }
        cc--;
    }
    bool ended(){
        return end;
    }
    std::string get_game_id(){
        return this->m_game_id;
    }
private:

    std::string get_game_view() {
        pt::ptree root;
        pt::ptree shapes;
        m_lock->lock();
        for (const auto &shape : asteroids) {
            pt::ptree child = shape->to_ptree();
            shapes.push_back(std::make_pair("", std::move(child)));
        }
        for (const auto &shape : vaisseaux) {
            pt::ptree child = shape->to_ptree();
            shapes.push_back(std::make_pair("", std::move(child)));
        }

        for (const auto &shape : vaisseaux2) {
            pt::ptree child = shape->to_ptree();
            shapes.push_back(std::make_pair("", std::move(child)));
        }

        for (laser &shape : lasers) {
            pt::ptree child = shape.to_ptree();
            shapes.push_back(std::make_pair("", std::move(child)));
        }
        m_lock->unlock();
        root.put("type", "game_view");
        root.add_child("shapes", shapes);
        std::stringstream ss;
        boost::property_tree::json_parser::write_json(ss, root);

        return ss.str();
    }

    pt::ptree get_infos() {
        pt::ptree root;
        root.put("type", "infos");

        int life_lev1 = -1;
        int life_lev2 = -1;


        root.put("s1", boost::lexical_cast<std::string>(m_score1));
        root.put("s2", boost::lexical_cast<std::string>(m_score2));
        int count1 = vaisseaux.size();
        int count2 = vaisseaux2.size();

        if (count1 > 0)
            life_lev1 = vaisseaux[0]->get_life_lev();

        if (count2 > 0)
            life_lev2 = vaisseaux2[0]->get_life_lev();

        return root;
    }

    std::string m_game_id;

private:
    std::vector<std::shared_ptr<asteroid>> asteroids;
    std::vector<laser> lasers;

    std::vector<std::shared_ptr<vaisseau>> vaisseaux;
    int m_score1;

    std::vector<std::shared_ptr<vaisseau>> vaisseaux2;
    int m_score2;

    int cc;
    bool end;
    bool m_not;
    std::shared_ptr<std::mutex> m_lock;
};

#endif //ASTEROID_GAME_H
