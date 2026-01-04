
#include <windows.h>
#include "psicrometria.h"
#include <math.h>

// Usiamo extern "C" per assicurarci che i nomi non vengano alterati dal compilatore C++
#ifdef __cplusplus
extern "C" {
#endif

	// --- FUNZIONI DI CONFIGURAZIONE ---

	__declspec(dllexport) double WINAPI Excel_set_quota(double altitude) {
		set_patm_at_altitude(altitude);
		return PATM;	}
	/*
	PSICRO_API Excel_set_quota(double altitude) {
		set_patm_at_altitude(altitude);
		return PATM; // Restituisce la pressione in kPa calcolata, così l'utente sa che ha funzionato
	} */

	// --- FUNZIONI BASE ---
	PSICRO_API Excel_Psat(double t) { return Psat(t); }
	PSICRO_API Excel_TPsat(double p_kpa) { return TPsat(p_kpa); }
	PSICRO_API Excel_xsat_t(double t) { return xsat_t(t); }

	// --- CALCOLO TEMPERATURA (T) ---
	PSICRO_API Excel_t_ur_x(double ur, double x) { return t_ur_x(ur, x); }
	PSICRO_API Excel_t_ur_h(double ur, double h) { return t_ur_h(ur, h); }
	PSICRO_API Excel_t_ur_vau(double ur, double vau) { return t_ur_vau(ur, vau); }
	PSICRO_API Excel_t_ur_tbu(double ur, double tbu) { return t_ur_tbu(ur, tbu); }
	PSICRO_API Excel_t_ur_tr(double ur, double tr) { return t_ur_tr(ur, tr); }
	PSICRO_API Excel_t_x_h(double x, double h) { return t_x_h(x, h); }
	PSICRO_API Excel_t_x_vau(double x, double vau) { return t_x_vau(x, vau); }
	PSICRO_API Excel_t_x_tbu(double x, double tbu) { return t_x_tbu(x, tbu); }
	PSICRO_API Excel_t_x_tr(double x, double tr) { return t_x_tr(x, tr); }
	PSICRO_API Excel_t_h_vau(double h, double vau) { return t_h_vau(h, vau); }
	PSICRO_API Excel_t_h_tbu(double h, double tbu) { return t_h_tbu(h, tbu); }
	PSICRO_API Excel_t_h_tr(double h, double tr) { return t_h_tr(h, tr); }
	PSICRO_API Excel_t_vau_tbu(double vau, double tbu) { return t_vau_tbu(vau, tbu); }
	PSICRO_API Excel_t_vau_tr(double vau, double tr) { return t_vau_tr(vau, tr); }
	PSICRO_API Excel_t_tbu_tr(double tbu, double tr) { return t_tbu_tr(tbu, tr); }

	// --- CALCOLO UMIDITÀ RELATIVA (UR) ---
	PSICRO_API Excel_ur_t_x(double t, double x) { return ur_t_x(t, x); }
	PSICRO_API Excel_ur_t_h(double t, double h) { return ur_t_h(t, h); }
	PSICRO_API Excel_ur_t_vau(double t, double vau) { return ur_t_vau(t, vau); }
	PSICRO_API Excel_ur_t_tbu(double t, double tbu) { return ur_t_tbu(t, tbu); }
	PSICRO_API Excel_ur_t_tr(double t, double tr) { return ur_t_tr(t, tr); }
	PSICRO_API Excel_ur_x_h(double x, double h) { return ur_x_h(x, h); }
	PSICRO_API Excel_ur_x_vau(double x, double vau) { return ur_x_vau(x, vau); }
	PSICRO_API Excel_ur_x_tbu(double x, double tbu) { return ur_x_tbu(x, tbu); }
	PSICRO_API Excel_ur_x_tr(double x, double tr) { return ur_x_tr(x, tr); }
	PSICRO_API Excel_ur_h_vau(double h, double vau) { return ur_h_vau(h, vau); }
	PSICRO_API Excel_ur_h_tbu(double h, double tbu) { return ur_h_tbu(h, tbu); }
	PSICRO_API Excel_ur_h_tr(double h, double tr) { return ur_h_tr(h, tr); }
	PSICRO_API Excel_ur_vau_tbu(double vau, double tbu) { return ur_vau_tbu(vau, tbu); }
	PSICRO_API Excel_ur_vau_tr(double vau, double tr) { return ur_vau_tr(vau, tr); }
	PSICRO_API Excel_ur_tbu_tr(double tbu, double tr) { return ur_tbu_tr(tbu, tr); }

	// --- CALCOLO UMIDITÀ SPECIFICA (X) ---
	PSICRO_API Excel_x_t_ur(double t, double ur) { return x_t_ur(t, ur); }
	PSICRO_API Excel_x_t_h(double t, double h) { return x_t_h(t, h); }
	PSICRO_API Excel_x_t_vau(double t, double vau) { return x_t_vau(t, vau); }
	PSICRO_API Excel_x_t_tbu(double t, double tbu) { return x_t_tbu(t, tbu); }
	PSICRO_API Excel_x_t_tr(double t, double tr) { return x_t_tr(t, tr); }
	PSICRO_API Excel_x_ur_h(double ur, double h) { return x_ur_h(ur, h); }
	PSICRO_API Excel_x_ur_vau(double ur, double vau) { return x_ur_vau(ur, vau); }
	PSICRO_API Excel_x_ur_tbu(double ur, double tbu) { return x_ur_tbu(ur, tbu); }
	PSICRO_API Excel_x_ur_tr(double ur, double tr) { return x_ur_tr(ur, tr); }
	PSICRO_API Excel_x_h_vau(double h, double vau) { return x_h_vau(h, vau); }
	PSICRO_API Excel_x_h_tbu(double h, double tbu) { return x_h_tbu(h, tbu); }
	PSICRO_API Excel_x_h_tr(double h, double tr) { return x_h_tr(h, tr); }
	PSICRO_API Excel_x_vau_tbu(double vau, double tbu) { return x_vau_tbu(vau, tbu); }
	PSICRO_API Excel_x_vau_tr(double vau, double tr) { return x_vau_tr(vau, tr); }
	PSICRO_API Excel_x_tbu_tr(double tbu, double tr) { return x_tbu_tr(tbu, tr); }

	// --- CALCOLO ENTALPIA (H) ---
	PSICRO_API Excel_h_t_ur(double t, double ur) { return h_t_ur(t, ur); }
	PSICRO_API Excel_h_t_x(double t, double x) { return h_t_x(t, x); }
	PSICRO_API Excel_h_t_vau(double t, double vau) { return h_t_vau(t, vau); }
	PSICRO_API Excel_h_t_tbu(double t, double tbu) { return h_t_tbu(t, tbu); }
	PSICRO_API Excel_h_t_tr(double t, double tr) { return h_t_tr(t, tr); }
	PSICRO_API Excel_h_ur_x(double ur, double x) { return h_ur_x(ur, x); }
	PSICRO_API Excel_h_ur_vau(double ur, double vau) { return h_ur_vau(ur, vau); }
	PSICRO_API Excel_h_ur_tbu(double ur, double tbu) { return h_ur_tbu(ur, tbu); }
	PSICRO_API Excel_h_ur_tr(double ur, double tr) { return h_ur_tr(ur, tr); }
	PSICRO_API Excel_h_x_vau(double x, double vau) { return h_x_vau(x, vau); }
	PSICRO_API Excel_h_x_tbu(double x, double tbu) { return h_x_tbu(x, tbu); }
	PSICRO_API Excel_h_x_tr(double x, double tr) { return h_x_tr(x, tr); }
	PSICRO_API Excel_h_vau_tbu(double vau, double tbu) { return h_vau_tbu(vau, tbu); }
	PSICRO_API Excel_h_vau_tr(double vau, double tr) { return h_vau_tr(vau, tr); }
	PSICRO_API Excel_h_tbu_tr(double tbu, double tr) { return h_tbu_tr(tbu, tr); }

	// --- CALCOLO VOLUME SPECIFICO (VAU) ---
	PSICRO_API Excel_vau_t_ur(double t, double ur) { return vau_t_ur(t, ur); }
	PSICRO_API Excel_vau_t_x(double t, double x) { return vau_t_x(t, x); }
	PSICRO_API Excel_vau_t_h(double t, double h) { return vau_t_h(t, h); }
	PSICRO_API Excel_vau_t_tbu(double t, double tbu) { return vau_t_tbu(t, tbu); }
	PSICRO_API Excel_vau_t_tr(double t, double tr) { return vau_t_tr(t, tr); }
	PSICRO_API Excel_vau_ur_x(double ur, double x) { return vau_ur_x(ur, x); }
	PSICRO_API Excel_vau_ur_h(double ur, double h) { return vau_ur_h(ur, h); }
	PSICRO_API Excel_vau_ur_tbu(double ur, double tbu) { return vau_ur_tbu(ur, tbu); }
	PSICRO_API Excel_vau_ur_tr(double ur, double tr) { return vau_ur_tr(ur, tr); }
	PSICRO_API Excel_vau_x_h(double x, double h) { return vau_x_h(x, h); }
	PSICRO_API Excel_vau_x_tbu(double x, double tbu) { return vau_x_tbu(x, tbu); }
	PSICRO_API Excel_vau_x_tr(double x, double tr) { return vau_x_tr(x, tr); }
	PSICRO_API Excel_vau_h_tbu(double h, double tbu) { return vau_h_tbu(h, tbu); }
	PSICRO_API Excel_vau_h_tr(double h, double tr) { return vau_h_tr(h, tr); }
	PSICRO_API Excel_vau_tbu_tr(double tbu, double tr) { return vau_tbu_tr(tbu, tr); }

	// --- CALCOLO BULBO UMIDO (TBU) ---
	PSICRO_API Excel_tbu_t_ur(double t, double ur) { return tbu_t_ur(t, ur); }
	PSICRO_API Excel_tbu_t_x(double t, double x) { return tbu_t_x(t, x); }
	PSICRO_API Excel_tbu_t_h(double t, double h) { return tbu_t_h(t, h); }
	PSICRO_API Excel_tbu_t_vau(double t, double vau) { return tbu_t_vau(t, vau); }
	PSICRO_API Excel_tbu_t_tr(double t, double tr) { return tbu_t_tr(t, tr); }
	PSICRO_API Excel_tbu_ur_x(double ur, double x) { return tbu_ur_x(ur, x); }
	PSICRO_API Excel_tbu_ur_h(double ur, double h) { return tbu_ur_h(ur, h); }
	PSICRO_API Excel_tbu_ur_vau(double ur, double vau) { return tbu_ur_vau(ur, vau); }
	PSICRO_API Excel_tbu_ur_tr(double ur, double tr) { return tbu_ur_tr(ur, tr); }
	PSICRO_API Excel_tbu_x_h(double x, double h) { return tbu_x_h(x, h); }
	PSICRO_API Excel_tbu_x_vau(double x, double vau) { return tbu_x_vau(x, vau); }
	PSICRO_API Excel_tbu_x_tr(double x, double tr) { return tbu_x_tr(x, tr); }
	PSICRO_API Excel_tbu_h_vau(double h, double vau) { return tbu_h_vau(h, vau); }
	PSICRO_API Excel_tbu_h_tr(double h, double tr) { return tbu_h_tr(h, tr); }
	PSICRO_API Excel_tbu_vau_tr(double vau, double tr) { return tbu_vau_tr(vau, tr); }

	// --- CALCOLO PUNTO DI RUGIADA (TR) ---
	PSICRO_API Excel_tr_t_ur(double t, double ur) { return tr_t_ur(t, ur); }
	PSICRO_API Excel_tr_t_x(double t, double x) { return tr_t_x(t, x); }
	PSICRO_API Excel_tr_t_h(double t, double h) { return tr_t_h(t, h); }
	PSICRO_API Excel_tr_t_vau(double t, double vau) { return tr_t_vau(t, vau); }
	PSICRO_API Excel_tr_t_tbu(double t, double tbu) { return tr_t_tbu(t, tbu); }
	PSICRO_API Excel_tr_ur_x(double ur, double x) { return tr_ur_x(ur, x); }
	PSICRO_API Excel_tr_ur_h(double ur, double h) { return tr_ur_h(ur, h); }
	PSICRO_API Excel_tr_ur_vau(double ur, double vau) { return tr_ur_vau(ur, vau); }
	PSICRO_API Excel_tr_ur_tbu(double ur, double tbu) { return tr_ur_tbu(ur, tbu); }
	PSICRO_API Excel_tr_x_h(double x, double h) { return tr_x_h(x, h); }
	PSICRO_API Excel_tr_x_vau(double x, double vau) { return tr_x_vau(x, vau); }
	PSICRO_API Excel_tr_x_tbu(double x, double tbu) { return tr_x_tbu(x, tbu); }
	PSICRO_API Excel_tr_h_vau(double h, double vau) { return tr_h_vau(h, vau); }
	PSICRO_API Excel_tr_h_tbu(double h, double tbu) { return tr_h_tbu(h, tbu); }
	PSICRO_API Excel_tr_vau_tbu(double vau, double tbu) { return tr_vau_tbu(vau, tbu); }

#ifdef __cplusplus
}
#endif