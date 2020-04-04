//
// Created by parallels on 4/4/20.
//

#ifndef ASTEROID_POLYCLIENT_H
#define ASTEROID_POLYCLIENT_H

#include "graphiqueSDL.h"
#include "polygone.h"
#include "affichable.h"

class polyClient : public polygone, public affichable
{
public:
    polyClient(std::initializer_list<point>&& liste): polygone(liste) {}
    explicit polyClient(std::vector<point> points): polygone(std::move(points)) {}
    void afficherSurFenetre(graphiqueSDL& fenetre) const override
    {
        const SDL_Color c = {255, 255,255,  255 };
        fenetre.dessinerPolyPlein(c, m_center, points);
    }
};

#endif //ASTEROID_POLYCLIENT_H
