#include "stdafx.h"
#include "Filter.h"
#include <math.h>

Filter::Filter ()
{
	_HDR_I[0] = _HDR_I[1] = _HDR_I[2] = 0;
	_HSR_I = 1.;

	_w_lp_init = false;
	_w_dl_init = false;

	_T = 0.005;
	_tau = 0.01;
}

Filter::~Filter ()
{
}

inline int SIGN (double v)
{
	if (v > 0.) return 1;
	if (v < 0.) return -1;
	return 0;
}

inline int SIGN_CHANGED (double v1, double v2)
{
	if (v1 < 0. && v2 > 0.) return 1;
	if (v1 > 0. && v2 < 0.) return 1;
	return 0;
}

void Filter::HDR (double v[3], double threshold, double ic)
{
	v[0] += _HDR_I[0];
	v[1] += _HDR_I[1];
	v[2] += _HDR_I[2];

	//const double ic = 0.001;
	//const double threshold = 15;

	if ((-threshold < v[0] && v[0] < threshold) && 
		(-threshold < v[1] && v[1] < threshold) && 
		(-threshold < v[2] && v[2] < threshold) ){

		_HDR_I[0] -= SIGN(v[0])*ic;
		_HDR_I[1] -= SIGN(v[1])*ic;
		_HDR_I[2] -= SIGN(v[2])*ic;
	}
}

void Filter::HSR (double v[3], double threshold, double ic)
{
	v[0] *= _HSR_I;
	v[1] *= _HSR_I;
	v[2] *= _HSR_I;

	double a = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]) - 1.;

	//const double ic = 1.0001;
	//const double threshold = 0.01;

	if (-threshold < a && a < threshold) {
		if (a > 0.)      _HSR_I /= ic;
		else if (a < 0.) _HSR_I *= ic;
	}
}

void Filter::LowpPassFilter (double v[3])
{
	if (!_w_lp_init) {
		_w_lp[0] = v[0];	
		_w_lp[1] = v[1];	
		_w_lp[2] = v[2];	

		_w_lp_init = true;
	}

	_w_lp[0] = (_T*v[0] + _tau*_w_lp[0])/(_T + _tau);
	_w_lp[1] = (_T*v[1] + _tau*_w_lp[1])/(_T + _tau);
	_w_lp[2] = (_T*v[2] + _tau*_w_lp[2])/(_T + _tau);

	v[0] = _w_lp[0];
	v[1] = _w_lp[1];
	v[2] = _w_lp[2];
}

void Filter::Delagging (double v[3])
{
	if (!_w_dl_init) {
		_w_dl[0] = v[0];	
		_w_dl[1] = v[1];	
		_w_dl[2] = v[2];	

		_w_dl_init = true;
	}

	double wd[3];

	wd[0] = v[0] + _tau/_T*(v[0] - _w_dl[0]);
	wd[1] = v[1] + _tau/_T*(v[1] - _w_dl[1]);
	wd[2] = v[2] + _tau/_T*(v[2] - _w_dl[2]);

	_w_dl[0] = v[0];
	_w_dl[1] = v[1];
	_w_dl[2] = v[2];

	v[0] = wd[0];
	v[1] = wd[1];
	v[2] = wd[2];
}

