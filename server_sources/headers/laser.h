
#ifndef ASTEROID_LASER_H
#define ASTEROID_LASER_H

#include "polyServeur.h"

class laser : public polyServeur {
public:
    laser(std::initializer_list<vec2d> &&liste) : polyServeur(liste) {}

    explicit laser(std::vector<vec2d> points) : polyServeur(std::move(points)) {}

    laser(vec2d &from, vec2d &to, int type = 1) : polyServeur({from, to}), m_type(type) {
        direction = vec2d{from.x - to.x, from.y - to.y};
        direction.normalize();
        direction *= 10;
    }

    bool isBordure() const {
        if (points[0].x >= LARGEUR) return true;
        if (points[0].x <= 0) return true;
        if (points[0].y >= HAUTEUR) return true;
        if (points[0].y <= 0) return true;
        return false;
    }

    int get_type() const {
        return m_type;
    }

    void set_type(int type) {
        m_type = type;
    }

    int m_type;
};

#endif //ASTEROID_LASER_H
