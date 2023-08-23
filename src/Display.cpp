#include "Display.hpp"

DEFINE_TYPE(TwitchSongRequest, Display);

void TwitchSongRequest::Display::Update() {
    if (!LayoutTransform && !Canvas) {
        return;
    }

    Scene currentScene = SceneManager::GetActiveScene();
    if(currentScene.IsValid()) {
        std::string Name = to_utf8(csstrtostr(currentScene.get_name()));
        auto position = getModConfig().PositionMenu.GetValue();
        auto rotation = getModConfig().RotationMenu.GetValue();

        if (Name == "GameCore") {
            position = getModConfig().PositionMenu.GetValue();
            rotation = getModConfig().RotationMenu.GetValue();
        }

        if (Name != "MainMenu") {
            if (Canvas) {
                Canvas->set_active(false);
            }
        }else {
            if (Canvas) {
                Canvas->set_active(true);
            }
        }

        SetPosition(position);
        SetRotation(rotation);

        if(Canvas) Canvas->GetComponent<RectTransform*>()->set_sizeDelta(Vector2(150.0f, 70.0f));

    }

    std::lock_guard<std::mutex> guard(mapObjectsMutex);
    for (auto it = mapObjects.begin(); it != mapObjects.end(); it++) {
        MapObject& object = *it;
        if (object.GameObject) {
            RectTransform* transform = object.GameObject->GetComponent<RectTransform*>();
            if ((transform->get_localPosition().y - transform->get_rect().get_yMax()) > Canvas->GetComponent<RectTransform*>()->get_sizeDelta().y) {
                Object::Destroy(object.GameObject);
                it = mapObjects.erase(it--);
            }
        } else {
            // Create a button with the song name and artist, which calls a function with the song id as a parameter
            std::string text = object.SongName + " - " + object.SongArtist;
            Button* button = BeatSaberUI::CreateUIButton(LayoutTransform, text, UnityEngine::Vector2(0.0f, 0.0f), UnityEngine::Vector2(150.0f, 10.0f), [this, object] () {
                // Remove the button from the list
                std::lock_guard<std::mutex> guard(mapObjectsMutex);
                for (auto it = mapObjects.begin(); it != mapObjects.end(); it++) {
                    MapObject& obj = *it;
                    if (obj.GameObject == object.GameObject) {
                        Object::Destroy(obj.GameObject);
                        it = mapObjects.erase(it--);
                    }
                }
                // Call the function to add the song to the queue
                DownloadSong(object.SongID);
            });

            object.GameObject = button->get_gameObject();
        }
    }
}

void TwitchSongRequest::Display::Finalize() {
    mapObjects.~vector();
}

void TwitchSongRequest::Display::SetPosition(UnityEngine::Vector3 position) {
    if(Canvas) {
        Canvas->GetComponent<RectTransform*>()->set_position(position);
    }
}

void TwitchSongRequest::Display::SetRotation(UnityEngine::Vector3 rotation) {
    if(Canvas) {
        Canvas->GetComponent<RectTransform*>()->set_eulerAngles(rotation);
    }
}

void TwitchSongRequest::Display::AddMapObject(MapObject object) {
    std::lock_guard<std::mutex> guard(mapObjectsMutex);
    mapObjects.push_back(object);
}

void TwitchSongRequest::Display::DownloadSong(std::string id) {
    getLogger().info("Downloading song with id: %s", id.c_str());
}