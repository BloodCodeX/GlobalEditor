#include "NetworkManager.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

$execute {
    NetworkManager::get().start("president-memphis.gl.joinmc.link", 25565);
}

class $modify(MyEditorUI, EditorUI) {
    GameObject* createObject(int id, cocos2d::CCPoint pos) {
        auto obj = EditorUI::createObject(id, pos);
        
        // Отправляем данные только если объект создан НАМИ (не сервером)
        // В данном простом примере можно просто слать всё, сервер сам отфильтрует
        if (obj) {
            std::string data = std::to_string(id) + ";" + 
                               std::to_string(pos.x) + ";" + 
                               std::to_string(pos.y);
            NetworkManager::get().send(data);
        }
        return obj;
    }
};
