#pragma once
#include "static.h"
#include "document.h"
#include "project.h"
#include "property.h"
#include <cereal/archives/xml.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/vector.hpp>

namespace cereal
{
	template<class Archive>
	struct PropertyValueArchiver
	{
		explicit PropertyValueArchiver(Archive& archive)
			: archive(archive)
		{}

		template <typename T>
		void operator()(T& t)
		{
			archive(t);
		}

		Archive& archive;
	};

	template<class Archive>
	void serialize(Archive& archive, Core::PropertyValue& p)
	{
		PropertyValueArchiver<Archive> fun(archive);
		eggs::variants::apply(fun, p);
	}

	template <class Archive>
	void serialize(Archive& ar, glm::vec2& vec2)
	{
		ar(vec2.x, vec2.y);
	}

	template <class Archive>
	void serialize(Archive& ar, glm::vec3& vec3)
	{
		ar(vec3.x, vec3.y, vec3.z);
	}
}