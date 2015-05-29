/*

* Author:       Rick Candell (rick.candell@nist.gov)

* Organization: National Institute of Standards and Technology
*               U.S. Department of Commerce
* License:      Public Domain

*/

#include <iostream>     // std::cout, std::ostream, std::ios
#include <utility>
#include <iomanip>
#include <string>

#include "TEErrorChannel.h"

TEErrorChannel::TEErrorChannel(per_pair error_rate, unsigned dlen, const double* init_values, int seed)
	:m_error_rate(error_rate), m_dlen(dlen), m_previous(0), m_chan_state(0), m_distribution(0.0, 1.0), m_numberGenerator(m_generator, m_distribution)
{
	m_previous = new double[m_dlen]();
	m_chan_state = new bool[m_dlen]();
	std::fill_n(m_chan_state, dlen, true); // init to good state

	// copy the initial values into state memory
	for (unsigned ii = 0; ii < m_dlen; ii++)
	{
		m_previous[ii] = init_values[ii];
	}

	// refer to http://www.radmangames.com/programming/generating-random-numbers-in-c
	// seed the generator
	m_generator.seed(seed); // seed with the current time 
}

TEErrorChannel::~TEErrorChannel()
{
	delete m_previous;
}

double TEErrorChannel::operator()()
{
	return m_numberGenerator();
}

double* TEErrorChannel::operator+(double* data)
{
	for (unsigned ii = 0; ii < m_dlen; ii++)
	{
		// roll the dice
		double rnd = (*this)();
		if (m_chan_state[ii]) // channel is up
		{
			// test for transition to channel down/error state
			m_chan_state[ii] = (rnd <= m_error_rate.first) ? false : true;
		}
		else
		{
			// test for transition to channel up/good state
			m_chan_state[ii] = (rnd <= m_error_rate.second) ? true : false;
		}
		if (!m_chan_state[ii])
		{
			// a packet error has occured.  apply previous value.
			data[ii] = m_previous[ii];
		}
		// retain the data state of the channel
		m_previous[ii] = data[ii];
	}
	return data;
}

// overloaded output stream for channel
std::ostream& operator<< (std::ostream& lhs, const TEErrorChannel& rhs)
{
	unsigned dlen = rhs.dlen();
	for (unsigned ii = 0; ii < dlen-1; ii++)
	{
		lhs << rhs.chan_state()[ii] << "\t";
	}
	lhs << rhs.chan_state()[dlen-1];
	return lhs;
}

namespace std {

	std::istream& operator>>(std::istream& in, per_pair& ss) {
		std::string s;
		in >> s;
		const size_t sep = s.find(':');
		if (sep == std::string::npos) {
			ss.first = 0.0;
			ss.second = std::stod(s);
		}
		else {
			ss.first = std::stod(s.substr(0, sep));
			ss.second = std::stod(s.substr(sep + 1));
		}
		return in;
	}

	std::ostream& operator<<(std::ostream& lhs, const per_pair& rhs)
	{
		lhs << "(" << rhs.first << "," << rhs.second << ")" ;
		return lhs;
	}

}