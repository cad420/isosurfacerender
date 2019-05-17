#include "ArcBall.h"


ysl::Point2i ArcBall::CartesianCoordInLeftBottom(const ysl::Point2i& screenPos)const
{
	return ysl::Point2i(screenPos.x,screenSize.y - screenPos.y);
}

ysl::Point2i ArcBall::CartesianCoordInCenter(const ysl::Point2i& screenPos) const
{
	const auto p = CartesianCoordInLeftBottom(screenPos);
	return ysl::Point2i( p.x - screenSize.x / 2,p.y - screenSize.y / 2 );
}

ysl::Point2f ArcBall::NormalizedCartesianCoordInCenter(const ysl::Point2i& screenPos) const
{
	return ysl::Point2f(1.0*screenPos.x / screenSize.x * 2 - 1.0,-(1.0*screenPos.y/screenSize.y*2-1.0));
}

ysl::Vector3f ArcBall::VecFromSphereCenter(const ysl::Point2i & screenPos) const
{
	const auto p = NormalizedCartesianCoordInCenter(screenPos);
	ysl::Vector3f v = { p.x,p.y,0.0 };
	const auto s = p.LengthSquared();
	if(s <=1)
		v.z = std::sqrt(1-s);
	else
		v.Normalize();
	return v;
}

ArcBall::ArcBall(int screenW, int screenH):ArcBall(ysl::Size2(screenW,screenH))
{

}

ArcBall::ArcBall(const ysl::Size2& screenSize):screenSize(screenSize)
{
}

void ArcBall::SetScreenSize(const ysl::Size2& screenSize)
{
	this->screenSize = screenSize;
}


float ArcBall::RotationRadiance(const ysl::Point2i & screenPos1, const ysl::Point2i& screenPos2) const
{
	auto v1 = VecFromSphereCenter(screenPos1), v2 = VecFromSphereCenter(screenPos2);
	v1.Normalize();
	v2.Normalize();
	return std::acos(ysl::Vector3f::Dot(v1, v2));
}

ysl::Vector3f ArcBall::RotationAxis(const ysl::Point2i& screenPos1, const ysl::Point2i& screenPos2) const
{
	const auto v1 = VecFromSphereCenter(screenPos1), v2 = VecFromSphereCenter(screenPos2);
	return  ysl::Vector3f::Cross(v1, v2);
}

ysl::Point2f ArcBall::NormalizedCartesianCoordInLeftBottom(const ysl::Point2i & screenPos) const
{
	const auto p = CartesianCoordInLeftBottom(screenPos);
	return { p.x / (float)screenSize.x,p.y / (float)screenSize.y };
}
