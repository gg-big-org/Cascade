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
*/

#ifndef NODEBASE_H
#define NODEBASE_H

#include <set>
#include <memory>

#include <QPen>

#include <gtest/gtest_prod.h>

#include "nodedefinitions.h"
#include "nodeproperties.h"
#include "../renderer/csimage.h"
#include "../windowmanager.h"
#include "../global.h"
#include "nodecontextmenu.h"
#include "nodegraphutility.h"

namespace Ui {
class NodeBase;
}

namespace Cascade {

using Renderer::CsImage;

class NodeInput;
class NodeOutput;
class NodeGraph;
class Connection;

class NodeBase : public QWidget
{
    Q_OBJECT

public:
    explicit NodeBase(
            const NodeType type,
            QWidget *parent = nullptr,
            const QString& isfName = "");

    bool operator==(const NodeBase* node) const;

    NodeType getType() const;
    bool getIsViewed() const;
    bool getNeedsUpdate() const;
    QSize getInputSize() const;
    QString getID() const;

    NodeProperties* getProperties() const;
    NodeInput* getNodeInputAtPosition(const QPoint pos);
    QString getAllPropertyValues() const;
    void getAllUpstreamNodes(std::vector<NodeBase*>& nodes);
    std::set<Connection*> getAllConnections();
    NodeInput* getOpenInput() const;

    void addNodeToJsonArray(QJsonArray& jsonNodesArray);

    NodeBase* getUpstreamNodeBack() const;
    NodeBase* getUpstreamNodeFront()const;
    NodeOutput* getRgbaOut() const;

    CsImage* getCachedImage() const;
    void setCachedImage(std::unique_ptr<CsImage> image);

    void invalidateAllDownstreamNodes();

    const std::vector<unsigned int>& getShaderCode();
    void setShaderCode(const std::vector<unsigned int> code);

    void loadNodePropertyValues(const NodePersistentProperties& p);

    NodeInput* findNodeInput(const QString& id);

    void flushCache();

    // Custom behavior in child classes
    virtual QSize getTargetSize() const;
    virtual bool canBeRendered() const;

    virtual ~NodeBase();

protected:
    void setLabeltext(const QString& text);

    std::unique_ptr<NodeProperties> mNodeProperties;
    Ui::NodeBase *ui;
    QString mIsfName = "";

private:
    FRIEND_TEST(NodeBaseTest, getAllDownstreamNodes_CorrectNumberOfNodes);
    FRIEND_TEST(NodeBaseTest, getAllDownstreamNodes_CorrectOrderOfNodes);
    FRIEND_TEST(NodeBaseTest, getAllUpstreamNodes_CorrectOrderOfNodes);

    // Init
    void setUpNode(const NodeType nodeType);
    void createInputs(const NodeInitProperties& props);
    void createOutputs(const NodeInitProperties& props);
    void setID(const QString& s);
    void setInputIDs(const QMap<int, QString>& ids);

    // Graph interaction
    void getAllDownstreamNodes(std::vector<NodeBase*>& nodes);
    void updateConnectionPositions();

    // Events
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void moveEvent(QMoveEvent*) override;

    void requestUpdate();
    void setNeedsUpdate(const bool b);

    const NodeType mNodeType;

    std::unique_ptr<CsImage> mCachedImage;

    NodeGraph* mNodeGraph;

    QString mId;

    std::vector<NodeInput*> mNodeInputs;
    std::vector<NodeOutput*> mNodeOutputs;

    NodeInput* mRgbaBackIn = nullptr;
    NodeInput* mRgbaFrontIn = nullptr;
    NodeOutput* mRgbaOut = nullptr;

    std::vector<unsigned int> mShaderCode;

    bool mNeedsUpdate = true;
    bool mIsSelected = false;
    bool mIsActive = false;
    bool mIsViewed = false;
    bool mIsDragging = false;

    QPoint mOldPos;

    const QBrush mDefaultColorBrush = QBrush(Config::sDefaultNodeColor);
    const QBrush mSelectedColorBrush = QBrush(Config::sSelectedNodeColor);

signals:
    void nodeWasLeftClicked(Cascade::NodeBase* node);
    void nodeWasRightClicked(Cascade::NodeBase* node);
    void nodeWasDoubleClicked(Cascade::NodeBase* node);
    void nodeRequestUpdate(Cascade::NodeBase* node);
    void nodeRequestFileSave(
            Cascade::NodeBase* node,
            const QString& path,
            const QString& fileType,
            const QMap<std::string, std::string>& attributes,
            const bool batchRender);
    void nodeHasMoved();

public slots:
    void handleSetSelected(Cascade::NodeBase* node, const bool b);
    void handleSetActive(Cascade::NodeBase* node, const bool b);
    void handleSetViewed(Cascade::NodeBase* node, const bool b);
    void handleRequestNodeUpdate();

    void onNodeHasBeenRendered(Cascade::NodeBase* node);
};

} // namespace Cascade


#endif // NODEBASE_H
