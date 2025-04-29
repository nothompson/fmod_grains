#pragma once
#include <memory> 
#include <vector>
#include <utility>
#include "fmod.hpp"

//will pirkle audio effects


//lerp with scaler/fraction
inline double doLinearInterpolation(double y1, double y2, double fractional_X)
{
	// check if invalid
	if (fractional_X >= 1.0) return y2;

	// lerp formula

	return fractional_X * y2 + (1.0 - fractional_X) * y1;
}

//ring buffer with wireAnd operations, more efficient than modulo for wrapping 

template <typename T>
class CircularBuffer
{
public:
	CircularBuffer() {}		//constructor, use once in the dsp state to initialize the buffers
	~CircularBuffer() {}	//destructor, use to 

	/** resetting all values to 0.0 */
	void flushBuffer() { memset(&buffer[0], 0, bufferLength * sizeof(T)); }

	//initialize in dspstate, dspcreate will link the state and its members functions on start
	//always 
	void createCircularBuffer(unsigned int _bufferLength)
	{
		// --- find nearest power of 2 for buffer, and create
		createCircularBufferPowerOfTwo((unsigned int)(pow(2, ceil(log(_bufferLength) / log(2)))));
	}

	/** Create a buffer based on a target maximum in SAMPLESwhere the size is
		pre-calculated as a power of two */


	void createCircularBufferPowerOfTwo(unsigned int _bufferLengthPowerOfTwo)
	{
		// --- reset to top
		writeIndex = 0;

		// --- find nearest power of 2 for buffer, save it as bufferLength
		bufferLength = _bufferLengthPowerOfTwo;

		// --- save (bufferLength - 1) for use as wrapping mask
		wrapMask = bufferLength - 1;

		// --- create new buffer
		buffer.reset(new T[bufferLength]);

		// --- flush buffer
		flushBuffer();
	}

	/** write a value into the buffer; this overwrites the previous oldest value in the buffer */
	void writeBuffer(T input)
	{
		// --- write and increment index counter
		buffer[writeIndex++] = input;

		// --- wrap if index > bufferlength - 1

		writeIndex &= wrapMask;
	}

	/** read an arbitrary location that is delayInSamples old */
	T readBuffer(int delayInSamples)//, bool readBeforeWrite = true)
	{
		// --- subtract to make read index
		//     note: -1 here is because we read-before-write,
		//           so the *last* write location is what we use for the calculation
		int readIndex = (writeIndex - 1) - delayInSamples;

		// --- autowrap index
		readIndex &= wrapMask;

		// --- read it
		return buffer[readIndex];
	}

	/** read an arbitrary location that includes a fractional sample */
	T readBuffer(double delayInFractionalSamples)
	{
		// --- truncate delayInFractionalSamples and read the int part
		T y1 = readBuffer((int)delayInFractionalSamples);

		// --- if no interpolation, just return value
		if (!interpolate) return y1;

		// --- else do interpolation
		//
		// --- read the sample at n+1 (one sample OLDER)
		T y2 = readBuffer((int)delayInFractionalSamples + 1);

		// --- get fractional part
		double fraction = delayInFractionalSamples - (int)delayInFractionalSamples;

		// --- do the interpolation (you could try different types here)
		return doLinearInterpolation(y1, y2, fraction);
	}

	unsigned int getWriteIndex() const {
		return writeIndex;
	}

	/** enable or disable interpolation; usually used for diagnostics or in algorithms that require strict integer samples times */
	void setInterpolate(bool b) { interpolate = b; }

private:
	std::unique_ptr<T[]> buffer = nullptr;	///< smart pointer will auto-delete
	unsigned int writeIndex = 0;		///> write index
	unsigned int bufferLength = 1024;	///< must be nearest power of 2
	unsigned int wrapMask = 1023;		///< must be (bufferLength - 1)
	bool interpolate = true;			///< interpolation (default is ON)
};

template <typename T> 
class Granulator
{
public: 
	Granulator() {}		//constructor
	~Granulator() {} //destructor

	//pointer to circular buffer that we are getting input from
	void setBuffer(CircularBuffer<T>* buffer) {
		circularBuffer = buffer;
	}

	void startGrain(double lengthinms, double start) {
		//conversion from ms to samples
		//divide ms by 1000, then multiply by samplerate
		double delay = (start / 1000.0) * 48000.0;
		double lengthinsamps = (lengthinms / 1000.0) * 48000.0;

		activeGrain.readPos = circularBuffer->getWriteIndex() + static_cast<int>(delay);
		activeGrain.lengthSamples = static_cast<int>(lengthinsamps);
		//initalize with zero
		activeGrain.index = 0;
		activeGrain.active = true;
	}

	T processGrain() {
		//return zeros if not active
		if (!activeGrain.active) {
			return T(0);
		}
		//when index has completed length, stop 
		if (activeGrain.index >= activeGrain.lengthSamples) {
			activeGrain.active = false;
			return T(0);
		}
			auto readPos = activeGrain.readPos;
			T outGrain = circularBuffer->readBuffer(readPos);

			//hann window
			double hann = 0.5 * (1.0 - cos(2.0 * PI * activeGrain.index / (activeGrain.lengthSamples - 1)));

			outGrain *= hann;

			activeGrain.index++;

			return outGrain;
	}

	bool isActive() const {
		return activeGrain.active;
	}



private:
	CircularBuffer<T>* circularBuffer = nullptr;
	unsigned int maxBufferSize = 1024;
	double PI = 3.141592653589793;

	struct Grain {
		double readPos = 0.0;
		int lengthSamples = 0;
		int index = 0;
		bool active = false;
	};

	Grain activeGrain;

	//std::vector<Grain> grainList;

};