
#pragma once
#include "../core/mathematics/geometry.h"
#include <functional>
#include <vector>
#include "../core/mathematics/spectrum.h"

namespace ysl
{

	enum class Color
	{
		green,
		red,
		blue,
		white,
		black,
		yellow,
		transparent,
		gray
	};

	inline RGBASpectrum TranslateColor(ysl::Color color)
	{
		float c[4];
		switch (color)
		{
		case Color::green:	(c[0] = 0, c[1] = 1, c[2] = 0, c[3] = 1); break;
		case Color::red:	(c[0] = 1, c[1] = 0, c[2] = 0, c[3] = 1); break;
		case Color::blue:	(c[0] = 0, c[1] = 0, c[2] = 1, c[3] = 1); break;
		case Color::white:	(c[0] = 1, c[1] = 1, c[2] = 1, c[3] = 1); break;
		case Color::yellow:	(c[0] = 1, c[1] = 1, c[2] = 0, c[3] = 1); break;
		case Color::black:	(c[0] = 0, c[1] = 0, c[2] = 0, c[3] = 1); break;
		case Color::transparent: (c[0] = 1, c[1] = 1, c[2] = 1, c[3] = 0); break;
		case Color::gray: (c[0] = 0.5, c[1] = 0.5, c[2] = 0.5, c[3] = 0.5); break;
		}
		return RGBASpectrum(c);
	}

	class MappingKey
	{
	public:
		Float intensity;
		RGBASpectrum leftColor;
		RGBASpectrum rightColor;
		MappingKey(Float intensity, const RGBASpectrum & lc, const RGBASpectrum & rc) :intensity(intensity), leftColor(lc), rightColor(rc) {}
		Float Intensity()const { return intensity; }
		RGBASpectrum LeftColor()const { return leftColor; }
		RGBASpectrum RightColor()const { return rightColor; }
		void SetIntensity(Float intensity) { intensity = intensity; }
		void SetLeftColor(const RGBASpectrum & c)
		{
			const auto a = leftColor.c[3];
			leftColor = c;
			leftColor.c[3] = a;

		}
		void SetRightColor(const RGBASpectrum & c)
		{
			const auto a = leftColor.c[3];
			rightColor = c;
			leftColor.c[3] = a;
		}
		void SetLeftAlpha(Float a) { leftColor.c[3] = a; }
		void SetRightAlpha(Float a) { rightColor.c[3] = a; }
	};

	class ColorInterpulator
	{
	public:
		ColorInterpulator() :m_valid(false) {}
		explicit ColorInterpulator(const std::string & fileName) :m_valid(false)
		{
			Read(fileName);
		}

		//template<typename ...Args>
		//ColorInterpulator(Args&&...args)
		//{
		//	
		//}

		void AddColorKey(float intensity, ysl::Color color);
		void AddColorKey(float intensity, const RGBASpectrum & spectrum);
		void Sort();
		void SetLeftThreshold(float left) { leftThreshold = left; }
		void SetRightThreshold(float right) { rightThreshold = right; }
		void SetLeftRightThreshold(float left, float right) { SetLeftThreshold(left); SetRightThreshold(right); }
		//template<typename  ...Args>
		//void SetColorKeys(Args&&... args);

		void Read(const std::string& fileName);
		bool valid()const
		{
			return m_valid;
		}
		void FetchData(RGBASpectrum* transferFunction, int dimension)const;
		void FetchData(Float * transferFunction, int dimension)const
		{
			FetchData(reinterpret_cast<RGBASpectrum*>(transferFunction), dimension);
		}
		int KeysCount()const
		{
			return keys.size();
		}
		//const MappingKey & Key()const
		//{
		//	
		//}

		const MappingKey & operator[](int i)const
		{
			return keys[i];
		}

		MappingKey & operator[](int i)
		{
			return keys[i];
		}

	private:
		bool m_valid;
	protected:
		std::vector<MappingKey> keys;
		Float leftThreshold = 0.0;
		Float rightThreshold = 1.0;
	};


	class TransferFunction :public ColorInterpulator
	{
	public:
		using Callback = std::function<void(TransferFunction* tf)>;
		TransferFunction() = default;
		TransferFunction(const std::string & fileName);
		~TransferFunction();
		void AddMappingKey(const MappingKey & key);
		void UpdateMappingKey(const Point2f & at, const Point2f & to, const Vector2f & scale = { 1.0f,1.0f });
		void UpdateMappingKey(const Point2f & at, const MappingKey & key, const Vector2f & scale = { 1.0f,1.0f });
		void AddUpdateCallBack(const Callback & func);
		Point2f KeyPosition(int i, Float sx, Float sy)const;
		int HitAt(const Point2f & at, const Vector2f & scale);
		void UpdateMappingKey(int index, const RGBASpectrum& color);
	private:
		std::vector<Callback> callbacks;
		std::function<bool(const MappingKey & m1, const MappingKey & m2)> compareFunc;
		void Notify();
		static Point2f TransToPos(const MappingKey & key);
	};
}
