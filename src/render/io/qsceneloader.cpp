/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsceneloader.h"
#include "qsceneloader_p.h"
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DCore/qscenepropertychange.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/private/renderlogging_p.h>
#include <QThread>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {

QSceneLoaderPrivate::QSceneLoaderPrivate()
    : QComponentPrivate()
    , m_status(QSceneLoader::Loading)
    , m_subTreeRoot(nullptr)
{
    m_shareable = false;
}


QSceneLoader::QSceneLoader(QNode *parent)
    : Qt3DCore::QComponent(*new QSceneLoaderPrivate, parent)
{
}

QSceneLoader::QSceneLoader(QSceneLoaderPrivate &dd, QNode *parent)
    : Qt3DCore::QComponent(dd, parent)
{
}

QSceneLoader::~QSceneLoader()
{
    QNode::cleanup();
}

// Called in main thread
void QSceneLoader::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &change)
{
    Q_D(QSceneLoader);
    QScenePropertyChangePtr e = qSharedPointerCast<QScenePropertyChange>(change);
    if (e->type() == NodeUpdated) {
        if (e->propertyName() == QByteArrayLiteral("scene")) {
            // If we already have a scene sub tree, delete it
            if (d->m_subTreeRoot) {
                delete d->m_subTreeRoot;
                d->m_subTreeRoot = nullptr;
            }

            // If we have successfully loaded a scene, graft it in
            auto *subTreeRoot = e->value().value<Qt3DCore::QEntity *>();
            if (subTreeRoot) {
                // Get the entity to which this component is attached
                const Qt3DCore::QNodeIdVector entities = d->m_scene->entitiesForComponent(d->m_id);
                Q_ASSERT(entities.size() == 1);
                Qt3DCore::QNodeId parentEntityId = entities.first();
                QEntity *parentEntity = qobject_cast<QEntity *>(d->m_scene->lookupNode(parentEntityId));
                subTreeRoot->setParent(parentEntity);
                d->m_subTreeRoot = subTreeRoot;
            }

            // Update status property
            setStatus(subTreeRoot ? QSceneLoader::Ready : QSceneLoader::Error);
        }
    }
}

void QSceneLoader::copy(const QNode *ref)
{
    Qt3DCore::QComponent::copy(ref);
    const QSceneLoader *s = static_cast<const QSceneLoader*>(ref);
    d_func()->m_source = s->d_func()->m_source;
}

QUrl QSceneLoader::source() const
{
    Q_D(const QSceneLoader);
    return d->m_source;
}

void QSceneLoader::setSource(const QUrl &arg)
{
    Q_D(QSceneLoader);
    if (d->m_source != arg) {
        d->m_source = arg;
        emit sourceChanged(arg);
    }
}

QSceneLoader::Status QSceneLoader::status() const
{
    Q_D(const QSceneLoader);
    return d->m_status;
}

void QSceneLoader::setStatus(QSceneLoader::Status status)
{
    Q_D(QSceneLoader);
    if (d->m_status != status) {
        d->m_status = status;
        emit statusChanged(status);
    }
}

Qt3DCore::QNodeCreatedChangeBasePtr QSceneLoader::createNodeCreationChange() const
{
    auto creationChange = Qt3DCore::QNodeCreatedChangePtr<QSceneLoaderData>::create(this);
    auto &data = creationChange->data;
    data.source = d_func()->m_source;
    return creationChange;
}

} // namespace Qt3DRender

QT_END_NAMESPACE
