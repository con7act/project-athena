//
//  HoverOverlayInterface.h
//  interface/src/ui/overlays
//
//  Created by Zach Fox on 2017-07-14.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_HoverOverlayInterface_h
#define hifi_HoverOverlayInterface_h

#include <QtCore/QObject>
#include <QUuid>

#include <DependencyManager.h>
#include <PointerEvent.h>
#include "EntityScriptingInterface.h"
#include "ui/overlays/Image3DOverlay.h"
#include "ui/overlays/Overlays.h"

#include "EntityTree.h"
#include "HoverOverlayLogging.h"

/**jsdoc
* @namespace HoverOverlay
*/
class HoverOverlayInterface : public QObject, public Dependency  {
    Q_OBJECT

    Q_PROPERTY(QUuid currentHoveredEntity READ getCurrentHoveredEntity WRITE setCurrentHoveredEntity)
    QSharedPointer<EntityScriptingInterface> _entityScriptingInterface;
    EntityPropertyFlags _entityPropertyFlags;
    OverlayID _hoverOverlayID { UNKNOWN_OVERLAY_ID };
    std::shared_ptr<Image3DOverlay> _hoverOverlay { nullptr };
public:
    HoverOverlayInterface();

    Q_INVOKABLE QUuid getCurrentHoveredEntity() { return _currentHoveredEntity; }
    void setCurrentHoveredEntity(const QUuid& entityID) { _currentHoveredEntity = entityID; }

public slots:
    void createHoverOverlay(const EntityItemID& entityItemID, const PointerEvent& event);
    void destroyHoverOverlay(const EntityItemID& entityItemID, const PointerEvent& event);
    void destroyHoverOverlay(const EntityItemID& entityItemID);
    void clickHoverOverlay(const OverlayID& overlayID, const PointerEvent& event);

private:
    bool _verboseLogging { true };
    QUuid _currentHoveredEntity {};

};

#endif // hifi_HoverOverlayInterface_h
