/****************************************************************************
**
** Copyright (C) 2015 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qservicelocator.h"
#include "qabstractserviceprovider_p.h"
#include "nullservices_p.h"
#include <QHash>

QT_BEGIN_NAMESPACE

namespace Qt3D {

/*!
    \class Qt3D::QAbstractServiceProvider
*/

QAbstractServiceProvider::QAbstractServiceProvider(int type, const QString &description)
    : d_ptr(new QAbstractServiceProviderPrivate(type, description))
{
}

/*! \internal */
QAbstractServiceProvider::QAbstractServiceProvider(QAbstractServiceProviderPrivate &dd)
    : d_ptr(&dd)
{
}

QAbstractServiceProvider::~QAbstractServiceProvider()
{
}

int QAbstractServiceProvider::type() const
{
    Q_D(const QAbstractServiceProvider);
    return d->m_type;
}

QString QAbstractServiceProvider::description() const
{
    Q_D(const QAbstractServiceProvider);
    return d->m_description;
}


class QServiceLocatorPrivate
{
public:
    QServiceLocatorPrivate()
        : m_nonNullDefaultServices(0)
    {}

    QHash<int, QAbstractServiceProvider *> m_services;

    NullSystemInformationService m_nullSystemInfo;
    NullOpenGLInformationService m_nullOpenGLInfo;
    int m_nonNullDefaultServices;
};


/*!
    \class Qt3D::QServiceLocator
    \inmodule Qt3DCore
    \brief Service locator used by aspects to retrieve pointers to concrete service objects

    The Qt3D::QServiceLocator class can be used by aspects to obtain pointers to concrete
    providers of abstract service interfaces. A subclass of Qt3D::QAbstractServiceProvider
    encapsulates a service that can be provided by an aspect for other parts of the system.
    For example, an aspect may wish to know the current frame number, or how many CPU cores
    are available in the Qt3D tasking threadpool.

    Aspects or the Qt3D::QAspectEngine are able to register objects as providers of services.
    The service locator itself can be accessed via the Qt3D::QAbstractAspect::services()
    function.

    As a convenience, the service locator provides methods to access services provided by
    built in Qt3D aspects. Currently these are Qt3D::QSystemInformationService and
    Qt3D::QOpenGLInformationService. For such services, the service provider will never
    return a null pointer. The default implementations of these services are simple null or
    do nothing implementations.
*/

/*!
    Creates an instance of QServiceLocator.
*/
QServiceLocator::QServiceLocator()
    : d_ptr(new QServiceLocatorPrivate)
{
}

/*!
   Destroys a QServiceLocator object
*/
QServiceLocator::~QServiceLocator()
{
}

/*!
    Registers \a provider service provider for the service \a serviceType. This replaces any
    existing provider for this service. The service provider does not take ownership
    of the provider.

    \sa unregisterServiceProvider(), serviceCount(), service()
*/
void QServiceLocator::registerServiceProvider(int serviceType, QAbstractServiceProvider *provider)
{
    Q_D(QServiceLocator);
    d->m_services.insert(serviceType, provider);
    if (serviceType < DefaultServiceCount)
        ++(d->m_nonNullDefaultServices);
}

/*!
    Unregisters any existing provider for the \a serviceType.
 */
void QServiceLocator::unregisterServiceProvider(int serviceType)
{
    Q_D(QServiceLocator);
    int removedCount = d->m_services.remove(serviceType);
    if (serviceType < DefaultServiceCount)
        d->m_nonNullDefaultServices -= removedCount;
}

/*!
    Returns the number of registered services.
 */
int QServiceLocator::serviceCount() const
{
    Q_D(const QServiceLocator);
    return DefaultServiceCount + d->m_services.size() - d->m_nonNullDefaultServices;
}

/*!
    \fn T *Qt3D::QServiceLocator::service(int serviceType)

    Returns a pointer to the service provider for \a serviceType. If no provider
    has been explicitly registered, this returns a null pointer for non-Qt3D provided
    default services and a null pointer for non-default services.

    \sa registerService()
*/

/*!
    Returns a pointer to a provider for the system information service. If no provider
    has been explicitly registered for this service type, then a pointer to a null, do-
    nothing service is returned.
*/
QSystemInformationService *QServiceLocator::systemInformation()
{
    Q_D(QServiceLocator);
    return static_cast<QSystemInformationService *>(d->m_services.value(SystemInformation, &d->m_nullSystemInfo));
}

/*!
    Returns a pointer to a provider for the OpenGL information service. If no provider
    has been explicitly registered for this service type, then a pointer to a null, do-
    nothing service is returned.
*/
QOpenGLInformationService *QServiceLocator::openGLInformation()
{
    Q_D(QServiceLocator);
    return static_cast<QOpenGLInformationService *>(d->m_services.value(OpenGLInformation, &d->m_nullOpenGLInfo));
}

/*!
    \internal
*/
QAbstractServiceProvider *QServiceLocator::_q_getServiceHelper(int type)
{
    Q_D(QServiceLocator);
    switch (type) {
    case SystemInformation:
        return systemInformation();
    case OpenGLInformation:
        return openGLInformation();
    default:
        return d->m_services.value(type, Q_NULLPTR);
    }
}

}

QT_END_NAMESPACE
