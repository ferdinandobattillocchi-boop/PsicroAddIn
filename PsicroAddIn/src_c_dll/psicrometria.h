#ifndef PSICROMETRIA_H
#define PSICROMETRIA_H

#include <math.h>

#ifdef _WIN32
	#include <windows.h>
// La macro include il tipo 'double' per essere usata come  - PSICRO_API NomeFunzione
	#define PSICRO_API __declspec(dllexport) double WINAPI
#else
	#define PSICRO_API double
#endif


// --- COSTANTI FONDAMENTALI ---
#define RAV         0.621945      // Rapporto masse molari Rav = Ra/Rv (Mw/Ma)
#define RA          0.287042      // Costante aria secca [kJ/kgK]
#define RV          0.461524      // Costante vapore acqueo [kJ/kgK]
// --- CAPACITÀ TERMICHE [kJ/kgK] ---
#define CPAS        1.006         // Aria secca
#define CPV         1.860         // Vapore
#define CPW         4.1868        // Acqua liquida
#define CPICE       2.090         // Ghiaccio
// --- CALORI LATENTI [kJ/kg] ---
#define LAMBDA      2501.0        // Evaporazione a 0°C
#define LAMBDA_ICE  333.4         // Fusione (Ghiaccio -> Liquido)
// --- PUNTI DI SWITCH ---
#define T_TRIPLO    0.01          // Punto triplo acqua [°C]
#define P_TRIPLO    0.611657      // Pressione punto triplo [kPa]

extern volatile double PATM;
__declspec(dllexport) void WINAPI set_patm_at_altitude(double altitude);
//void get_patm_at_altitude(double altitude);
PSICRO_API Psat(double t);
PSICRO_API TPsat(double p_kpa);
PSICRO_API xsat_t(double t);
PSICRO_API stima_iniziale_t(double p_kpa);
PSICRO_API t_ur_x(double ur, double x);
PSICRO_API t_ur_h(double ur, double h);
PSICRO_API t_ur_vau(double ur, double vau);
PSICRO_API t_ur_tbu(double ur, double tbu);
PSICRO_API t_ur_tr(double ur, double tr);
PSICRO_API t_x_h(double x, double h);
PSICRO_API t_x_vau(double x, double vau);
PSICRO_API t_x_tbu(double x, double tbu);
PSICRO_API t_x_tr(double x, double tr);
PSICRO_API t_h_vau(double h, double vau);
PSICRO_API t_h_tbu(double h, double tbu);
PSICRO_API t_h_tr(double h, double tr);
PSICRO_API t_vau_tbu(double vau, double tbu);
PSICRO_API t_vau_tr(double vau, double tr);
PSICRO_API t_tbu_tr(double tbu, double tr);
PSICRO_API ur_t_x(double t, double x);
PSICRO_API ur_t_h(double t, double h);
PSICRO_API ur_t_vau(double t, double vau);
PSICRO_API ur_t_tbu(double t, double tbu);
PSICRO_API ur_t_tr(double t, double tr);
PSICRO_API ur_x_h(double x, double h);
PSICRO_API ur_x_vau(double x, double vau);
PSICRO_API ur_x_tbu(double x, double tbu);
PSICRO_API ur_x_tr(double x, double tr);
PSICRO_API ur_h_vau(double h, double vau);
PSICRO_API ur_h_tbu(double h, double tbu);
PSICRO_API ur_h_tr(double h, double tr);
PSICRO_API ur_vau_tbu(double vau, double tbu);
PSICRO_API ur_vau_tr(double vau, double tr);
PSICRO_API ur_tbu_tr(double tbu, double tr);
PSICRO_API x_t_ur(double t, double ur);
PSICRO_API x_t_h(double t, double h);
PSICRO_API x_t_vau(double t, double vau);
PSICRO_API x_t_tbu(double t, double tbu);
PSICRO_API x_t_tr(double t, double tr);
PSICRO_API x_ur_h(double ur, double h);
PSICRO_API x_ur_vau(double ur, double vau);
PSICRO_API x_ur_tbu(double ur, double tbu);
PSICRO_API x_ur_tr(double ur, double tr);
PSICRO_API x_h_vau(double h, double vau);
PSICRO_API x_h_tbu(double h, double tbu);
PSICRO_API x_h_tr(double h, double tr);
PSICRO_API x_vau_tbu(double vau, double tbu);
PSICRO_API x_vau_tr(double vau, double tr);
PSICRO_API x_tbu_tr(double tbu, double tr);
PSICRO_API h_t_ur(double t, double ur);
PSICRO_API h_t_x(double t, double x);
PSICRO_API h_t_vau(double t, double vau);
PSICRO_API h_t_tbu(double t, double tbu);
PSICRO_API h_t_tr(double t, double tr);
PSICRO_API h_ur_x(double ur, double x);
PSICRO_API h_ur_vau(double ur, double vau);
PSICRO_API h_ur_tbu(double ur, double tbu);
PSICRO_API h_ur_tr(double ur, double tr);
PSICRO_API h_x_vau(double x, double vau);
PSICRO_API h_x_tbu(double x, double tbu);
PSICRO_API h_x_tr(double x, double tr);
PSICRO_API h_vau_tbu(double vau, double tbu);
PSICRO_API h_vau_tr(double vau, double tr);
PSICRO_API h_tbu_tr(double tbu, double tr);
PSICRO_API vau_t_ur(double t, double ur);
PSICRO_API vau_t_x(double t, double x);
PSICRO_API vau_t_h(double t, double h);
PSICRO_API vau_t_tbu(double t, double tbu);
PSICRO_API vau_t_tr(double t, double tr);
PSICRO_API vau_ur_x(double ur, double x);
PSICRO_API vau_ur_h(double ur, double h);
PSICRO_API vau_ur_tbu(double ur, double tbu);
PSICRO_API vau_ur_tr(double ur, double tr);
PSICRO_API vau_x_h(double x, double h);
PSICRO_API vau_x_tbu(double x, double tbu);
PSICRO_API vau_x_tr(double x, double tr);
PSICRO_API vau_h_tbu(double h, double tbu);
PSICRO_API vau_h_tr(double h, double tr);
PSICRO_API vau_tbu_tr(double tbu, double tr);
PSICRO_API tbu_t_ur(double t, double ur);
PSICRO_API tbu_t_x(double t, double x);
PSICRO_API tbu_t_h(double t, double h);
PSICRO_API tbu_t_vau(double t, double vau);
PSICRO_API tbu_t_tr(double t, double tr);
PSICRO_API tbu_ur_x(double ur, double x);
PSICRO_API tbu_ur_h(double ur, double h);
PSICRO_API tbu_ur_vau(double ur, double vau);
PSICRO_API tbu_ur_tr(double ur, double tr);
PSICRO_API tbu_x_h(double x, double h);
PSICRO_API tbu_x_vau(double x, double vau);
PSICRO_API tbu_x_tr(double x, double tr);
PSICRO_API tbu_h_vau(double h, double vau);
PSICRO_API tbu_h_tr(double h, double tr);
PSICRO_API tbu_vau_tr(double vau, double tr);
PSICRO_API tr_t_ur(double t, double ur);
PSICRO_API tr_t_x(double t, double x);
PSICRO_API tr_t_h(double t, double h);
PSICRO_API tr_t_vau(double t, double vau);
PSICRO_API tr_t_tbu(double t, double tbu);
PSICRO_API tr_ur_x(double ur, double x);
PSICRO_API tr_ur_h(double ur, double h);
PSICRO_API tr_ur_vau(double ur, double vau);
PSICRO_API tr_ur_tbu(double ur, double tbu);
PSICRO_API tr_x_h(double x, double h);
PSICRO_API tr_x_vau(double x, double vau);
PSICRO_API tr_x_tbu(double x, double tbu);
PSICRO_API tr_h_vau(double h, double vau);
PSICRO_API tr_h_tbu(double h, double tbu);
PSICRO_API tr_vau_tbu(double vau, double tbu);
#endif