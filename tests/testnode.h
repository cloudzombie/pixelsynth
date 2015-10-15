#pragma once
#include "static.h"

struct TestNode
{
	using p = PropertyMetadata::Builder;
	using c = ConnectorMetadata::Builder;

	static PropertyMetadataCollection propertyMetadata()
	{
		return
		{
			p("$Title").ofType<std::string>().build(),
			p("int").ofType<int>().build(),
			p("double").ofType<double>().build(),
			p("vec2").ofType<glm::vec2>().build(),
			p("vec3").ofType<glm::vec3>().build(),
			p("string").ofType<std::string>().build()
		};
	}

	static ConnectorMetadataCollection connectorMetadata()
	{
		return
		{
			c("Out", ConnectorType::Output).build(),
			c("In", ConnectorType::Input).build()
		};
	}

	static Metadata* metadata()
	{
		static auto m = Metadata
		{
			propertyMetadata(),
			connectorMetadata()
		};
		return &m;
	}

	static void addKeyframes(Document::Builder& mut, NodePtr n)
	{
		mut.mutate(n, [&](Node::Builder& node)
		{
			node.mutateProperty(hash("int"), [&](Property::Builder& prop) { prop.set(0, -500); prop.set(100, 500); });
			node.mutateProperty(hash("double"), [&](Property::Builder& prop) { prop.set(0, -500.0); prop.set(100, 500.0); });
			node.mutateProperty(hash("vec2"), [&](Property::Builder& prop) { prop.set(0, glm::vec2(-500, 50)); prop.set(100, glm::vec2(500, -50)); });
			node.mutateProperty(hash("vec3"), [&](Property::Builder& prop) { prop.set(0, glm::vec3(-500, 50, 100)); prop.set(100, glm::vec3(500, -50, 0)); });
			node.mutateProperty(hash("string"), [&](Property::Builder& prop) { prop.set(0, "a"); prop.set(100, "b"); });
		});
	}

	static void assertKeyframes(NodePtr n)
	{
		AssertThat(prop(*n, "int")->get<int>(0), Equals(-500));
		AssertThat(prop(*n, "int")->get<int>(50), Equals(0));
		AssertThat(prop(*n, "int")->get<int>(100), Equals(500));

		AssertThat(prop(*n, "double")->get<double>(0), Equals(-500));
		AssertThat(prop(*n, "double")->get<double>(50), Equals(0));
		AssertThat(prop(*n, "double")->get<double>(100), Equals(500));

		AssertThat(prop(*n, "vec2")->get<glm::vec2>(0), Equals(glm::vec2(-500, 50)));
		AssertThat(prop(*n, "vec2")->get<glm::vec2>(50), Equals(glm::vec2(0, 0)));
		AssertThat(prop(*n, "vec2")->get<glm::vec2>(100), Equals(glm::vec2(500, -50)));

		AssertThat(prop(*n, "vec3")->get<glm::vec3>(0), Equals(glm::vec3(-500, 50, 100)));
		AssertThat(prop(*n, "vec3")->get<glm::vec3>(50), Equals(glm::vec3(0, 0, 50)));
		AssertThat(prop(*n, "vec3")->get<glm::vec3>(100), Equals(glm::vec3(500, -50, 0)));

		AssertThat(prop(*n, "string")->get<std::string>(0), Equals("a"));
		AssertThat(prop(*n, "string")->get<std::string>(50), Equals("a"));
		AssertThat(prop(*n, "string")->get<std::string>(100), Equals("b"));
	}
};
