#ifndef ALIA_QT_LAYOUT_HPP
#define ALIA_QT_LAYOUT_HPP

#include <QLayout>
#include <QWidget>

// This file attempts to reimagine Qt's dual widget/layout object hierarchies
// as a single hierarchy of UI objects that's more compatible with alia's
// interface conventions and object tree management capabilities.
//
// More documentation coming once I figure out how this is actually going to
// work...
//

struct layout_container;

// class QLayout;
// class QLayoutItem;
class QWidget;

struct layout_node
{
    void
    remove();

    virtual void
    relocate(
        layout_container& new_parent, layout_node* after, layout_node* before)
        = 0;

    // Get the index of this node in the parent layout container.
    // (:parent must be non-null when this is called.)
    virtual int
    index() const = 0;

    layout_container*
    parent() const
    {
        return parent_;
    }

 protected:
    layout_container* parent_ = nullptr;
};

struct layout_container : layout_node
{
    int
    index() const override
    {
        return this->parent()->layout()->indexOf(this->layout());
    }

    void
    relocate(
        layout_container& new_parent,
        layout_node* after,
        layout_node* before) override
    {
        new_parent.insert_layout(this->layout(), after, before);
        parent_ = &new_parent;
    }

    virtual QLayout*
    layout() const = 0;

    void
    remove(layout_node* node)
    {
        this->layout()->takeAt(node->index());
    }

    virtual void
    insert_widget(QWidget* widget, layout_node* after, layout_node* before)
        = 0;

    virtual void
    insert_layout(QLayout* layout, layout_node* after, layout_node* before)
        = 0;
};

struct widget_layout_node : layout_node
{
    void
    initialize(QWidget* widget)
    {
        widget_ = widget;
    }

    int
    index() const override
    {
        return this->parent()->layout()->indexOf(this->widget_);
    }

    void
    relocate(
        layout_container& new_parent,
        layout_node* after,
        layout_node* before) override
    {
        new_parent.insert_widget(widget_, after, before);
        parent_ = &new_parent;
    }

 private:
    QWidget* widget_ = nullptr;
};

struct box_layout : layout_container
{
    void
    initialize(QBoxLayout* layout)
    {
        layout_ = layout;
    }

    virtual QLayout*
    layout() const override
    {
        return layout_;
    }

    void
    insert_widget(
        QWidget* object, layout_node* after, layout_node* before) override
    {
        layout_->insertWidget(before ? before->index() : -1, object);
    }

    void
    insert_layout(
        QLayout* object, layout_node* after, layout_node* before) override
    {
        layout_->insertLayout(before ? before->index() : -1, object);
    }

 private:
    QBoxLayout* layout_;
};

struct layout_object
{
    layout_node* node;

    void
    remove()
    {
        node->remove();
    }

    void
    relocate(
        layout_object& new_parent, layout_object* after, layout_object* before)
    {
        node->relocate(
            static_cast<layout_container&>(*new_parent.node),
            after ? after->node : nullptr,
            before ? before->node : nullptr);
    }
};

#endif
