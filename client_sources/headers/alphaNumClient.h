
#ifndef ASTEROID_ALPHANUMCLIENT_H
#define ASTEROID_ALPHANUMCLIENT_H

#include <string>
#include <utility>
#include "affichable.h"
#include "graphiqueSDL.h"
#include "point.h"

class alphaNumClient : public affichable {
public:

    alphaNumClient() : pos{10, 10} {};

    void afficherSurFenetre(graphiqueSDL &fenetre) const override {
        fenetre.dessinerTexte(val, pos());
    }

    void changeTexte(const std::string &tireX, const std::string &tireV, const std::string &scoreEquipe,
                     const std::string &viesEquipe) {
        val = "SCORE EQUIPE: " + scoreEquipe;
        val += "    VIES EQUIPE: " + viesEquipe;
        val += "    TIRE C: " + tireX;
        val += "    TIRE V: " + tireV;
    }

private:
    std::string val;
    vec2d pos;
};

#endif //ASTEROID_ALPHANUMCLIENT_H
