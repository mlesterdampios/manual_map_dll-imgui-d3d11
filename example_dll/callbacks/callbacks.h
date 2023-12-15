#pragma once

#include <cstdlib>
#include <unordered_map>
#include <functional>

class CALLBACKS_BASE {

public:
	std::unordered_map <std::string, std::function<void(void)>> OnPresentCallBacks;
	std::unordered_map <std::string, std::function<void(void)>> OnSlowUpdateCallBacks;
	std::unordered_map <std::string, std::function<void(void)>> OnImguiInitCallBacks;
	std::unordered_map <std::string, std::function<void(void)>> TabItemsContent;


	virtual void inline AddOnPresentCallBacks(std::string name, std::function<void(void)> fn) {};
	virtual void inline RemoveOnPresentCallBacks(std::string name) {};
	virtual void inline DeleteAllOnPresentCallBacks() {};

	virtual void inline AddOnImguiInitCallBacks(std::string name, std::function<void(void)> fn) {};
	virtual void inline RemoveOnImguiInitCallBacks(std::string name) {};
	virtual void inline DeleteAllOnImguiInitCallBacks() {};

	virtual void inline AddTabItemsContent(std::string name, std::function<void(void)> fn) {};
	virtual void inline RemoveTabItemsContent(std::string name) {};
	virtual void inline DeleteAllTabItemsContent() {};

	virtual void inline AddOnSlowUpdateCallBacks(std::string name, std::function<void(void)> fn) {};
	virtual void inline RemoveOnSlowUpdateCallBacks(std::string name) {};
	virtual void inline DeleteAllOnSlowUpdateCallBacks() {};
};

class CALLBACKS final : public CALLBACKS_BASE {
private:
	
public:
	void AddOnPresentCallBacks(std::string name, std::function<void(void)> fn) override;
	void RemoveOnPresentCallBacks(std::string name) override;
	void DeleteAllOnPresentCallBacks() override;

	void AddOnImguiInitCallBacks(std::string name, std::function<void(void)> fn) override;
	void RemoveOnImguiInitCallBacks(std::string name) override;
	void DeleteAllOnImguiInitCallBacks() override;

	void AddTabItemsContent(std::string name, std::function<void(void)> fn) override;
	void RemoveTabItemsContent(std::string name) override;
	void DeleteAllTabItemsContent() override;

	void AddOnSlowUpdateCallBacks(std::string name, std::function<void(void)> fn) override;
	void RemoveOnSlowUpdateCallBacks(std::string name) override;
	void DeleteAllOnSlowUpdateCallBacks() override;
};

inline CALLBACKS_BASE* CALLBACKS_INSTANCE;