#pragma once
#include "static.h"
#include "document.h"
#include "project.h"
#include "property.h"
#include <cereal/archives/xml.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

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
}