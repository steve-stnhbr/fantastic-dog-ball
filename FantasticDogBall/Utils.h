#pragma once
#include <string>
#include <stdexcept>
#include <GL/glew.h>
#include <glm/ext/matrix_transform.hpp>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <vector >

class Globals {
public:
	static unsigned NUM_POINT_LIGHTS;
	static unsigned NUM_DIRECTIONAL_LIGHTS;
	static unsigned NUM_SPOT_LIGHTS;
	static unsigned NUM_SHADOW_CUBEMAPS;
	static unsigned NUM_SHADOW_MAPS;
	static unsigned WINDOW_WIDTH;
	static unsigned WINDOW_HEIGHT;
	static unsigned NUM_LEVELS;
};

class Utils
{
public:
	static std::string readFile(const char* path);
	static GLenum checkError_(const char* file, int line);
	static std::string getISOCurrentTimestamp();
	static void start2D();
	static void end2D();
	static inline std::string arr2str(const std::vector<std::string> v) {
		std::string str;
		for (const auto& s : v) {
			if (s.empty()) continue;
			str += s + ", ";
		}

		auto l = str.length();
		return str.substr(0, l - 2);
	}
	static inline constexpr unsigned int str2int(const char* str, int h = 0)
	{
		return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
	}

	static void strToLower(std::string);
	static std::vector<std::string> filesInDirectory(std::string);
	static inline glm::mat3 toGLM(btMatrix3x3 mat) {
		return glm::mat3(
			mat[0][0], mat[0][1], mat[0][2],
			mat[1][0], mat[1][1], mat[1][2],
			mat[2][0], mat[2][1], mat[2][2]
		);
	}

	static inline btMatrix3x3 toBT(glm::mat3 mat) {
		return btMatrix3x3(
			mat[0][0], mat[0][1], mat[0][2],
			mat[1][0], mat[1][1], mat[1][2],
			mat[2][0], mat[2][1], mat[2][2]
		);
	}

	template<typename ... Args>
	static std::string string_format(const std::string& format, Args ... args)
	{
		int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		if (size_s <= 0) { throw std::logic_error("Error during formatting."); }
		auto size = static_cast<size_t>(size_s);
		char* buf(new char[size]);
		std::snprintf(buf, size, format.c_str(), args ...);
		return std::string(buf, buf + size - 1); // We don't want the '\0' inside
	}

	template<typename Base, typename T>
	static bool instanceof(const T*) {
		return std::is_base_of<Base, T>::value;
	}
	static void CheckDebugLog();
	static void DebugOutputToFile(unsigned int source, unsigned int type, unsigned int id,
		unsigned int severity, const char* message);
	static inline float getAngle(float y, float x) {
		const unsigned short roundPlaces = 6;
		y = Utils::round(y, roundPlaces);
		x = Utils::round(x, roundPlaces);
		// todo make right calculation
		/*
		auto angle = atan2(y, x);
		if (y < 0) angle = glm::two_pi<float>() + angle;
		*/
		auto angle = glm::mod(atan2(y, x) + glm::two_pi<float>(), glm::two_pi<float>());
		return angle;
	}

	template <typename T> static inline int sgn(T val) {
		return (T(0) < val) - (val < T(0));
	}

	template <typename T>
	static inline T round(const T val, int places) {
		const auto roundFactor = pow(10, places);
		return floor(val * roundFactor) / roundFactor;
	}

	#define checkError() checkError_(__FILE__, __LINE__) 
	
	template <typename K, typename V>
	struct Map {
		std::vector<K> keys;
		std::vector<V> vals;

		Map() {
			keys = std::vector<K>(0);
			vals = std::vector<V>(0);
		}

		size_t insert(K key, V val) {
			size_t index = keys.size();

			auto find = std::find(keys.begin(), keys.end(), key);

			if (find != keys.end()) {
				index = find - keys.begin();
				vals[index] = (val);
			}
			else {
				keys.push_back(key);
				vals.push_back(val);
			}
			return index;
		}

		V get(K key) const {
			for (auto i = 0; i < keys.size(); i++) {
				if (keys[i] == key)
					return vals[i];
			}
			return NULL;
		}

		bool contains(K key) {
			return std::count(keys.begin(), keys.end(), key);
		}

		bool contains(V val) {
			return std::count(vals.begin(), vals.end(), val);
		}
	};
};
