#pragma once
#include "mathematics/geometry.h"

class ArcBall
{
	ysl::Size2 screenSize;
public:
	ArcBall(int screenW, int screenH);
	ArcBall(const ysl::Size2 & screenSize);
	void SetScreenSize(const ysl::Size2 & screenSize);
	float RotationRadiance(const ysl::Point2i & screenPos1, const ysl::Point2i& screenPos2)const;
	ysl::Vector3f RotationAxis(const ysl::Point2i& screenPos1, const ysl::Point2i& screenPos2)const;
	ysl::Point2i CartesianCoordInLeftBottom(const ysl::Point2i & screenPos)const;
	ysl::Point2i CartesianCoordInCenter(const ysl::Point2i & screenPos)const;
	ysl::Point2f NormalizedCartesianCoordInLeftBottom(const ysl::Point2i & screenPos)const;
	ysl::Point2f NormalizedCartesianCoordInCenter(const ysl::Point2i& screenPos)const;
	ysl::Vector3f VecFromSphereCenter(const ysl::Point2i & screenPos)const;
	ArcBall()=default;
	~ArcBall()=default;
};
