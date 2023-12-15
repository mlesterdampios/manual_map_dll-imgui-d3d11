#pragma once

class MAINMENU_BASE {
public:
	virtual inline void ShenMenu() {};
	virtual inline bool getIsShowMenu() { return false; };
};

class MAINMENU final : public MAINMENU_BASE {
private:
	bool isShowMenu = true;
	int menuKey = VK_INSERT;
public:
	void ShenMenu() override;
	bool getIsShowMenu() override;
};

inline MAINMENU_BASE *MAINMENU_INSTANCE;