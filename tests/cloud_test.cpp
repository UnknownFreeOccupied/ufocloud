// UFO
#include <ufo/pcl/cloud.hpp>

// Catch2
#include <catch2/catch_test_macros.hpp>

struct Point {
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;

	Point() = default;

	Point(float x, float y, float z) : x(x), y(y), z(z) {}

	void operator+=(Point rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
	}
};

struct Occupancy {
	float occupancy = 0.5;

	Occupancy() = default;

	Occupancy(float occupancy) : occupancy(occupancy) {}

	Occupancy& operator=(float occupancy)
	{
		this->occupancy = occupancy;
		return *this;
	}
};

struct Color {
	float red   = 0.0;
	float green = 0.0;
	float blue  = 0.0;

	Color() = default;

	Color(float red, float green, float blue) : red(red), green(green), blue(blue) {}
};

std::ostream& operator<<(std::ostream& out, Point p)
{
	return out << "X: " << p.x << " Y: " << p.y << " Z: " << p.z;
}

std::ostream& operator<<(std::ostream& out, Occupancy o)
{
	return out << "Occupancy: " << o.occupancy;
}

std::ostream& operator<<(std::ostream& out, Color c)
{
	return out << "Red: " << c.red << " Green: " << c.green << " Blue: " << c.blue;
}

template <class... Ts, std::enable_if_t<(std::is_same_v<Point, Ts> || ...), bool> = true>
void applyTranslation(ufo::Cloud<Ts...>& cloud, Point const& translation)
{
	for (auto e : cloud) {
		static_cast<Point&>(e) += translation;
	}
}

TEST_CASE("Cloud")
{
	ufo::Cloud<Occupancy, Color, Point> cloud;

	cloud.emplace_back(0.5, Color{1.0, 0.0, 1.0});
	cloud.emplace_back();
	cloud.emplace_back(3.0);

	decltype(cloud)::value_type value = cloud[0];

	std::cout << value << std::endl;

	std::cout << cloud[0] << std::endl;

	std::cout << static_cast<Occupancy>(cloud[0]) << std::endl;

	cloud[0].get<Occupancy>() = 0.75;

	std::cout << static_cast<Occupancy>(cloud[0]) << std::endl;

	cloud[0].set<Occupancy>(1.0);

	std::cout << static_cast<Occupancy>(cloud[0]) << std::endl;

	static_cast<Occupancy&>(cloud[0]) = 0.3;

	std::cout << static_cast<Occupancy>(cloud[0]) << std::endl;

	std::cout << cloud[0].get<Color>() << std::endl;

	cloud[0].set<Color>(1.0, 0.3, 0.0);

	std::cout << cloud[0].get<Color>() << std::endl;

	std::cout << std::as_const(cloud)[0].occupancy << std::endl;

	decltype(cloud)::value_type ce = cloud[0];

	std::cout << ce.occupancy << std::endl;

	cloud.push_back(cloud[0]);

	cloud[1].set<Occupancy>(0.0);

	std::cout << cloud[0] << std::endl;
	std::cout << cloud[1] << std::endl;

	std::cout << "Before:" << std::endl;
	for (auto const& e : cloud) {
		std::cout << e << std::endl;
	}

	for (auto e : cloud) {
		e.set<Color>(0.0, 0.0, 0.0);
	}

	std::cout << "After:" << std::endl;
	for (auto const& e : cloud) {
		std::cout << e << std::endl;
	}

	ufo::Cloud<Point, Color> c1(10);

	std::cout << "Before" << std::endl;
	for (auto const& e : c1) {
		std::cout << e << std::endl;
	}

	applyTranslation(c1, Point(1, -2, 5));

	std::cout << "After" << std::endl;
	for (auto const& e : c1) {
		std::cout << e << std::endl;
	}

	ufo::Cloud<Color> c2(5);

	std::cout << "Before" << std::endl;
	for (auto const& e : c2) {
		std::cout << e << std::endl;
	}

	// applyTranslation(c2, Point(1, -2, 5));

	std::cout << "After" << std::endl;
	for (auto const& e : c2) {
		std::cout << e << std::endl;
	}
}