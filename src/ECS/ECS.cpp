#include "ECS.h"
#include "../Logger/Logger.h"
#include <vector>
#include <algorithm>

int IComponent::m_nextId = 0;

// Entity ##########################################################################
int Entity::GetId() const {
    return m_id;
}

void Entity::Kill() {
    m_registry->KillEntity(*this);
}

void Entity::Tag(const std::string& tag) {
        m_registry->TagEntity(*this, tag);
}

bool Entity::HasTag(const std::string& tag) const {
        return m_registry->EntityHasTag(*this, tag);
}

void Entity::Group(const std::string& group) {
        m_registry->GroupEntity(*this, group);
}

bool Entity::BelongsToGroup(const std::string& group) const {
        return m_registry->EntityBelongsToGroup(*this, group);
}


// System ##########################################################################

void System::AddEntityToSystem(Entity entity) {
    m_entities.push_back(entity);
}

void System::RemoveEntityFromSystem(Entity entity) {
    m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(), [&entity](Entity other) {
        return entity == other;
    }), m_entities.end());
}

std::vector<Entity> System::GetSystemEntities() const {
    return m_entities;
}

const Signature& System::GetComponentSignature() const {
    return m_componentSignature;
}

// Registry ##########################################################################
Entity Registry::CreateEntity(){
    
    int entityId;

    if (m_freeIds.empty()){
        // If there are no free id's waiting to be reused, then we create and resize.
        entityId = m_numEntities++;

        //Make sure the entityComponentSignatures vector can accomodate the new entity
        if (entityId >= static_cast<int>(m_entityComponentSignatures.size())) {
            m_entityComponentSignatures.resize(entityId + 1);
        }
    }
    // Reuse an id from the list previously removed
    else {
        entityId = m_freeIds.front();
        m_freeIds.pop_front();
        Logger::Log("Entity ID reused for creation with id = " + std::to_string(entityId));
    }

    Entity entity(entityId);
    entity.m_registry = this;

    // Flagging new entity to be added
    m_entitiesTobeAdded.insert(entity);

    
    //Logger::Log("Entity flagged for creation with id = " + std::to_string(entityId));
    return entity;
    
};

void Registry::KillEntity(Entity entity) {
    m_entitiesToBeKilled.insert(entity);
}

void Registry::AddEntityToSystems(Entity entity) {
    const auto entityId = entity.GetId();

    // Component signatures for that entity
    const auto entityComponentSignature = m_entityComponentSignatures[entityId];

    // Loop all the system
    for (auto& system: m_systems) {
        const auto& systemComponentSignature = system.second->GetComponentSignature();

        // Check each bit from one bitset with another, if all of them match we add that entity to the system.
        bool isInterested = (entityComponentSignature & systemComponentSignature) == systemComponentSignature;
        if (isInterested) {
            // Add the entity to the system
            system.second->AddEntityToSystem(entity);
        }
    }
}

void Registry::RemoveEntityFromSystems(Entity entity) {
    for (auto system: m_systems) {
        system.second->RemoveEntityFromSystem(entity);
    };

}

void Registry::TagEntity(Entity entity, const std::string& tag) {
    entityPerTag.emplace(tag, entity);
    tagPerEntity.emplace(entity.GetId(), tag);
}

bool Registry::EntityHasTag(Entity entity, const std::string& tag) const {
        if (tagPerEntity.find(entity.GetId()) == tagPerEntity.end()) {
            return false;
        }
        return entityPerTag.find(tag)->second == entity;
}

Entity Registry::GetEntityByTag(const std::string& tag) const {
    return entityPerTag.at(tag);
}

void Registry::RemoveEntityTag(Entity entity) {
        auto taggedEntity = tagPerEntity.find(entity.GetId());
    if (taggedEntity != tagPerEntity.end()) {
        auto tag = taggedEntity->second;
        entityPerTag.erase(tag);
        tagPerEntity.erase(taggedEntity);
    }
}

void Registry::GroupEntity(Entity entity, const std::string& group) {
    entitiesPerGroup.emplace(group, std::set<Entity>());
    entitiesPerGroup[group].emplace(entity);
    groupPerEntity.emplace(entity.GetId(), group);
}

bool Registry::EntityBelongsToGroup(Entity entity, const std::string& group) const {
    // If do not find
    if (entitiesPerGroup.find(group) == entitiesPerGroup.end()) {
        return false;
    }
    auto groupEntities = entitiesPerGroup.at(group);
    return groupEntities.find(entity.GetId()) != groupEntities.end();
}

std::vector<Entity> Registry::GetEntitiesByGroup(const std::string& group) const {
    auto& setOfEntities = entitiesPerGroup.at(group);
    return std::vector<Entity>(setOfEntities.begin(), setOfEntities.end());
}

void Registry::RemoveEntityGroup(Entity entity) {
    // if in group, remove entity from group management
    auto groupedEntity = groupPerEntity.find(entity.GetId());
    if (groupedEntity != groupPerEntity.end()) {
        auto group = entitiesPerGroup.find(groupedEntity->second);
        if (group != entitiesPerGroup.end()) {
            auto entityInGroup = group->second.find(entity);
            if (entityInGroup != group->second.end()) {
                group->second.erase(entityInGroup);
            }
        }
        groupPerEntity.erase(groupedEntity);
    }
}

void Registry::Update() {
    // processing the entitites that are waiting to be CREATED to the active system
    for(auto entity: m_entitiesTobeAdded) {
        Registry::AddEntityToSystems(entity);
    }
    m_entitiesTobeAdded.clear();

    // Process the entities that are waiting to be killed from the active systems
    for(auto entity: m_entitiesToBeKilled) {

        Registry::RemoveEntityFromSystems(entity);

        // Clear the component signatures of that entity
        m_entityComponentSignatures[entity.GetId()].reset();

        // Remove the entity from the component pools
        for (auto pool: m_componentTypePools) {
            if (pool) {
                pool->RemoveEntityFromPool(entity.GetId());
            }
        }

        // Make the entity id to be available to be reused
        m_freeIds.push_back(entity.GetId());

        // Remove any traces of that entity from the tag/group maps
        RemoveEntityTag(entity);
        RemoveEntityGroup(entity);
    }
    m_entitiesToBeKilled.clear();
}