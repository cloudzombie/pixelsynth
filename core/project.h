#pragma once
#include "static.h"
#include "document.h"

BEGIN_NAMESPACE(Core)

class Project
{
public:
	struct UndoState
	{
		std::string undoDescription;
		std::string redoDescription;
		bool canUndo;
		bool canRedo;

		UndoState(const std::string& undoDescription, const std::string& redoDescription, const bool canUndo, const bool canRedo)
			: undoDescription(undoDescription),
			  redoDescription(redoDescription),
			  canUndo(canUndo),
			  canRedo(canRedo)
		{
		}
	};

	using history_group_t = std::pair<std::string, Document>; // description + changes
	using history_t = std::vector<history_group_t>;
	using redohistory_t = std::stack<history_group_t>;
	using mutate_fn = std::function<void(Document::Builder&)>;
	using mutation_callback_fn = std::function<void(std::shared_ptr<MutationInfo>)>;

	Project();

	NodePtr root() const noexcept { return root_; }

	void undo() noexcept;
	void redo() noexcept;
	UndoState undoState() const noexcept;

	const Document& current() const noexcept;
	void mutate(mutate_fn fn, std::string description = "") noexcept;
	void mutate(std::initializer_list<mutate_fn> fns, std::string description = "") noexcept;

	void setMutationCallback(mutation_callback_fn fn) noexcept;

private:	
	friend class cereal::access;
	template<class Archive> void save(Archive& archive) const;
	template<class Archive>	void load(Archive& archive);

	history_t history_;
	redohistory_t redoStack_;
	NodePtr root_;
	mutation_callback_fn mutationCallback_;
};

END_NAMESPACE(Core)