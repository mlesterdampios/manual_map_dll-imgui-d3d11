#include "callbacks.h"

void CALLBACKS::AddOnPresentCallBacks(std::string name, std::function<void(void)> fn) {
	OnPresentCallBacks[name] = fn;
}

void CALLBACKS::RemoveOnPresentCallBacks(std::string name) {
	OnPresentCallBacks.erase(name);
}

void CALLBACKS::DeleteAllOnPresentCallBacks() {
	OnPresentCallBacks.clear();
}

void CALLBACKS::AddOnSlowUpdateCallBacks(std::string name, std::function<void(void)> fn) {
	OnSlowUpdateCallBacks[name] = fn;
}

void CALLBACKS::RemoveOnSlowUpdateCallBacks(std::string name) {
	OnSlowUpdateCallBacks.erase(name);
}

void CALLBACKS::DeleteAllOnSlowUpdateCallBacks() {
	OnSlowUpdateCallBacks.clear();
}

void CALLBACKS::AddOnImguiInitCallBacks(std::string name, std::function<void(void)> fn) {
	OnImguiInitCallBacks[name] = fn;
}

void CALLBACKS::RemoveOnImguiInitCallBacks(std::string name) {
	OnImguiInitCallBacks.erase(name);
}

void CALLBACKS::DeleteAllOnImguiInitCallBacks() {
	OnImguiInitCallBacks.clear();
}

void CALLBACKS::AddTabItemsContent(std::string name, std::function<void(void)> fn) {
	TabItemsContent[name] = fn;
}

void CALLBACKS::RemoveTabItemsContent(std::string name) {
	TabItemsContent.erase(name);
}

void CALLBACKS::DeleteAllTabItemsContent() {
	TabItemsContent.clear();
}