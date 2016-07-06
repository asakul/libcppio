
#ifndef CPPIO_ERRORS_H
#define CPPIO_ERRORS_H

namespace cppio
{
	enum ErrorCode
	{
		eTimeout = -1,
		eConnectionLost = -2,
		eTooBigBuffer = -3,
		eUnknown = -100
	};
}

#endif /* ifndef CPPIO_ERRORS_H */
