/*
 *  Cascade Image Editor
 *
 *  Copyright (C) 2022 Till Dechent and contributors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  NodeEditor code adapted from:
 *  Dmitry Pinaev et al, Qt Node Editor, (2017), GitHub repository, https://github.com/paceholder/nodeeditor
*/

#include "nodegraphview.h"

#include <QtWidgets/QGraphicsScene>

#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtWidgets/QMenu>

#include <QtCore/QRectF>
#include <QtCore/QPointF>

#include <QtWidgets>

#include <QDebug>
#include <iostream>
#include <cmath>

#include "nodegraphscene.h"
#include "datamodelregistry.h"
#include "node.h"
#include "nodegraphicsobject.h"
#include "connectiongraphicsobject.h"
#include "stylecollection.h"
#include "contextmenu.h"

#include "../log.h"

using Cascade::NodeGraph::NodeGraphView;
using Cascade::NodeGraph::NodeGraphScene;

NodeGraphView::NodeGraphView(QWidget *parent) :
    QGraphicsView(parent),
    mClearSelectionAction(Q_NULLPTR),
    mDeleteSelectionAction(Q_NULLPTR),
    mScene(Q_NULLPTR)
{
    setDragMode(QGraphicsView::RubberBandDrag);
    setRenderHint(QPainter::Antialiasing);

    auto const &nodeGraphViewStyle = StyleCollection::nodeGraphViewStyle();

    setBackgroundBrush(nodeGraphViewStyle.BackgroundColor);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    setCacheMode(QGraphicsView::CacheBackground);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    auto scene = new NodeGraphScene(registerDataModels(), this);

    setScene(scene);

    mContextMenu = new ContextMenu(scene, this);

    scale(0.7, 0.7);

    connect(mScene, &NodeGraphScene::nodeDoubleClicked,
            this, &NodeGraphView::setActiveNode);
}


QAction* NodeGraphView::clearSelectionAction() const
{
    return mClearSelectionAction;
}


QAction* NodeGraphView::deleteSelectionAction() const
{
    return mDeleteSelectionAction;
}


void NodeGraphView::setScene(NodeGraphScene *scene)
{
    mScene = scene;
    QGraphicsView::setScene(mScene);

    // setup actions
    delete mClearSelectionAction;
    mClearSelectionAction = new QAction(QStringLiteral("Clear Selection"), this);
    mClearSelectionAction->setShortcut(Qt::Key_Escape);
    connect(mClearSelectionAction, &QAction::triggered, mScene, &QGraphicsScene::clearSelection);
    addAction(mClearSelectionAction);

    delete mDeleteSelectionAction;
    mDeleteSelectionAction = new QAction(QStringLiteral("Delete Selection"), this);
    mDeleteSelectionAction->setShortcut(Qt::Key_Delete);
    connect(mDeleteSelectionAction, &QAction::triggered, this, &NodeGraphView::deleteSelectedNodes);
    addAction(mDeleteSelectionAction);
}


void NodeGraphView::contextMenuEvent(QContextMenuEvent *event)
{
    if (itemAt(event->pos()))
    {
        QGraphicsView::contextMenuEvent(event);
        return;
    }

    QPoint pos = event->pos();

    mContextMenu->exec(this->mapToScene(pos).toPoint());
}


void NodeGraphView::wheelEvent(QWheelEvent *event)
{
    QPoint delta = event->angleDelta();

    if (delta.y() == 0)
    {
        event->ignore();
        return;
    }

    double const d = delta.y() / std::abs(delta.y());

    if (d > 0.0)
        scaleUp();
    else
        scaleDown();
}


void NodeGraphView::scaleUp()
{
    double const step   = 1.2;
    double const factor = std::pow(step, 1.0);

    QTransform t = transform();

    if (t.m11() > 2.0)
        return;

    scale(factor, factor);
}


void NodeGraphView::scaleDown()
{
    double const step   = 1.2;
    double const factor = std::pow(step, -1.0);

    scale(factor, factor);
}


void NodeGraphView::deleteSelectedNodes()
{
    // Delete the selected connections first, ensuring that they won't be
    // automatically deleted when selected nodes are deleted (deleting a node
    // deletes some connections as well)
    for (QGraphicsItem * item : mScene->selectedItems())
    {
        if (auto c = qgraphicsitem_cast<ConnectionGraphicsObject*>(item))
            mScene->deleteConnection(c->connection());
    }

    // Delete the nodes; this will delete many of the connections.
    // Selected connections were already deleted prior to this loop, otherwise
    // qgraphicsitem_cast<NodeGraphicsObject*>(item) could be a use-after-free
    // when a selected connection is deleted by deleting the node.
    for (QGraphicsItem * item : mScene->selectedItems())
    {
        if (auto n = qgraphicsitem_cast<NodeGraphicsObject*>(item))
            mScene->removeNode(n->node());
    }
}


void NodeGraphView::setActiveNode(Node* node)
{
    mActiveNode = node;

    emit activeNodeChanged(node);
}


void NodeGraphView::handleFrontViewRequested()
{
    CS_LOG_INFO("BEEP");
}


void NodeGraphView::handleBackViewRequested()
{

}


void NodeGraphView::handleAlphaViewRequested()
{

}


void NodeGraphView::handleResultViewRequested()
{

}


void NodeGraphView::keyPressEvent(QKeyEvent *event)
{
    QGraphicsView::keyPressEvent(event);
}


void NodeGraphView::keyReleaseEvent(QKeyEvent *event)
{
    QGraphicsView::keyReleaseEvent(event);
}


void NodeGraphView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    if (event->button() == Qt::MiddleButton)
    {
        setDragMode(QGraphicsView::ScrollHandDrag);
        mMiddleClickPos = mapToScene(event->pos());
    }
}


void NodeGraphView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    if (scene()->mouseGrabberItem() == nullptr && event->buttons() == Qt::MiddleButton)
    {
        QPointF difference = mMiddleClickPos - mapToScene(event->pos());
        setSceneRect(sceneRect().translated(difference.x(), difference.y()));
    }
}


void NodeGraphView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);

    setDragMode(QGraphicsView::RubberBandDrag);
}


void NodeGraphView::drawBackground(QPainter* painter, const QRectF& r)
{
    QGraphicsView::drawBackground(painter, r);

//    auto drawGrid =
//        [&](double gridStep)
//    {
//        QRect   windowRect = rect();
//        QPointF tl = mapToScene(windowRect.topLeft());
//        QPointF br = mapToScene(windowRect.bottomRight());

//        double left   = std::floor(tl.x() / gridStep - 0.5);
//        double right  = std::floor(br.x() / gridStep + 1.0);
//        double bottom = std::floor(tl.y() / gridStep - 0.5);
//        double top    = std::floor (br.y() / gridStep + 1.0);

//        // vertical lines
//        for (int xi = int(left); xi <= int(right); ++xi)
//        {
//            QLineF line(xi * gridStep, bottom * gridStep,
//                        xi * gridStep, top * gridStep );

//            painter->drawLine(line);
//        }

//        // horizontal lines
//        for (int yi = int(bottom); yi <= int(top); ++yi)
//        {
//            QLineF line(left * gridStep, yi * gridStep,
//                        right * gridStep, yi * gridStep );
//            painter->drawLine(line);
//        }
//    };

//    auto const &nodeGraphViewStyle = StyleCollection::nodeGraphViewStyle();

//    QBrush bBrush = backgroundBrush();

//    QPen pfine(nodeGraphViewStyle.FineGridColor, 1.0);

//    painter->setPen(pfine);
//    drawGrid(15);

//    QPen p(nodeGraphViewStyle.CoarseGridColor, 1.0);

//    painter->setPen(p);
//    drawGrid(150);
}


void NodeGraphView::showEvent(QShowEvent *event)
{
    mScene->setSceneRect(this->rect());
    QGraphicsView::showEvent(event);
}


NodeGraphScene* NodeGraphView::scene()
{
    return mScene;
}
