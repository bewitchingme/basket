/***************************************************************************
 *   Copyright (C) 2003 by Sébastien Laoût                                 *
 *   slaout@linux62.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef NOTECONTENT_H
#define NOTECONTENT_H

#include <QtCore/QObject>
#include <QGraphicsItem>
#include <QUrl>

#include <KIO/AccessManager>

#include <phonon/phononnamespace.h>

#include "linklabel.h"

class QDomDocument;
class QDomElement;

class QBuffer;
class QColor;
class QMimeData;
class QMovie;
class QPainter;
class QPixmap;
class QPoint;
class QRect;
class QString;
class QStringList;
class QTextDocument;
class QWidget;

class KFileItem;
class QUrl;

namespace KIO
{
    class PreviewJob;
}

namespace Phonon
{
    class MediaObject;
}

class BasketScene;
class FilterData;
class Note;

/**
 * LinkDisplayItem is a QGraphicsItem using a LinkDisplay
 */
class LinkDisplayItem : public QGraphicsItem
{
public:
  LinkDisplayItem(Note *parent):m_note(parent) {}
  virtual ~LinkDisplayItem() {}
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
  
  LinkDisplay &linkDisplay() { return m_linkDisplay; }
private:
  LinkDisplay m_linkDisplay;
  Note *m_note;
};

/** A list of numeric identifier for each note type.
  * Declare a varible with the type NoteType::Id and assign a value like NoteType::Text...
  * @author Sébastien Laoût
  */
namespace NoteType
{
enum Id { Group = 255, Text = 1, Html, Image, Animation, Sound, File, Link, CrossReference, Launcher, Color, Unknown }; // Always positive
}

/** Abstract base class for every content type of basket note.
 * It's a base class to represent those types: Text, Html, Image, Animation, Sound, File, Link, Launcher, Color, Unknown.
 * @author Sébastien Laoût
 */
class NoteContent
{
public:
    // Constructor and destructor:
    explicit NoteContent(Note *parent, const QString &fileName = "");     /// << Constructor. Inherited notes should call it to initialize the parent note.
    virtual ~NoteContent()                                             {} /// << Virtual destructor. Reimplement it if you should destroy some data your custom types.
    // Simple Abstract Generic Methods:
    virtual NoteType::Id type() const                                = 0; /// << @return the internal number that identify that note type.
    virtual QString typeName() const                                 = 0; /// << @return the translated type name to display in the user interface.
    virtual QString lowerTypeName() const                            = 0; /// << @return the type name in lowercase without space, for eg. saving.
    virtual QString toText(const QString &cuttedFullPath);                /// << @return a plain text equivalent of the content.
    virtual QString toHtml(const QString &imageName, const QString &cuttedFullPath) = 0; /// << @return an HTML text equivalent of the content. @param imageName Save image in this Qt resource.
    virtual QPixmap toPixmap()                      {
        return QPixmap();
    } /// << @return an image equivalent of the content.
    virtual void    toLink(QUrl *url, QString *title, const QString &cuttedFullPath); /// << Set the link to the content. By default, it set them to fullPath() if useFile().
    virtual bool    useFile() const                                  = 0; /// << @return true if it use a file to store the content.
    virtual bool    canBeSavedAs() const                             = 0; /// << @return true if the content can be saved as a file by the user.
    virtual QString saveAsFilters() const                            = 0; /// << @return the filters for the user to choose a file destination to save the note as.
    virtual bool    match(const FilterData &data)                    = 0; /// << @return true if the content match the filter criterias.
    // Complexe Abstract Generic Methods:
    virtual void exportToHTML(HTMLExporter *exporter, int indent)    = 0; /// << Export the note in an HTML file.
    virtual QString cssClass() const                                 = 0; /// << @return the CSS class of the note when exported to HTML
    virtual qreal     setWidthAndGetHeight(qreal width)                  = 0; /// << Relayout content with @p width (never less than minWidth()). @return its new height.
    virtual bool    loadFromFile(bool /*lazyLoad*/)     {
        return false;
    } /// << Load the content from the file. The default implementation does nothing. @see fileName().
    virtual bool    finishLazyLoad()                    {
        return false;
    } /// << Load what was not loaded by loadFromFile() if it was lazy-loaded
    virtual bool    saveToFile()                        {
        return false;
    } /// << Save the content to the file. The default implementation does nothing. @see fileName().
    virtual QString linkAt(const QPointF &/*pos*/)          {
        return "";
    } /// << @return the link anchor at position @p pos or "" if there is no link.
    virtual void    saveToNode(QDomDocument &doc, QDomElement &content);  /// << Save the note in the basket XML file. By default it store the filename if a file is used.
    virtual void    fontChanged()                                    = 0; /// << If your content display textual data, called when the font have changed (from tags or basket font)
    virtual void    linkLookChanged()                                  {} /// << If your content use LinkDisplay with preview enabled, reload the preview (can have changed size)
    virtual QString editToolTipText() const                          = 0; /// << @return "Edit this [text|image|...]" to put in the tooltip for the note's content zone.
    virtual void    toolTipInfos(QStringList */*keys*/, QStringList */*values*/) {} /// << Get "key: value" couples to put in the tooltip for the note's content zone.
    // Custom Zones:                                                      ///    Implement this if you want to store custom data.
    virtual int	zoneAt(const QPointF &/*pos*/)           {
        return 0;
    } /// << If your note-type have custom zones, @return the zone at @p pos or 0 if it's not a custom zone!
    virtual QRectF   zoneRect(int /*zone*/, const QPointF &/*pos*/);            /// << Idem, @return the rect of the custom zone
    virtual QString zoneTip(int /*zone*/)                  {
        return "";
    } /// << Idem, @return the toolTip of the custom zone
    virtual Qt::CursorShape cursorFromZone(int /*zone*/) const   { return Qt::ArrowCursor; } /// << Idem, @return the mouse cursor when it is over zone @p zone!
    virtual void    setHoveredZone(int /*oldZone*/, int /*newZone*/)   {} /// << If your note type need some feedback, you get notified of hovering changes here.
    virtual QString statusBarMessage(int /*zone*/)         {
        return "";
    } /// << @return the statusBar message to show for zone @p zone, or "" if nothing special have to be said.
    // Drag and Drop Content:
    virtual void    serialize(QDataStream &/*stream*/)                 {} /// << Serialize the content in a QDragObject. If it consists of a file, it can be serialized for you.
    virtual bool    shouldSerializeFile()           {
        return useFile();
    } /// << @return true if the dragging process should serialize the filename (and move the file if cutting).
    virtual void    addAlternateDragObjects(QMimeData */*dragObj*/) {} /// << If you offer more than toText/Html/Image/Link(), this will be called if this is the only selected.
    virtual QPixmap feedbackPixmap(qreal width, qreal height)            = 0; /// << @return the pixmap to put under the cursor while dragging this object.
    virtual bool    needSpaceForFeedbackPixmap()        {
        return false;
    } /// << @return true if a space must be inserted before and after the DND feedback pixmap.
    // Content Edition:
    virtual int xEditorIndent()                        {
        return 0;
    } /// << If the editor should be indented (eg. to not cover an icon), return the number of pixels.
    // Open Content or File:
    virtual QUrl urlToOpen(bool /*with*/); /// << @return the URL to open the note, or an invalid QUrl if it's not openable. If @p with if false, it's a normal "Open". If it's true, it's for an "Open with..." action. The default implementation return the fullPath() if the note useFile() and nothing if not.
    enum OpenMessage {
        OpenOne,              /// << Message to send to the statusbar when opening this note.
        OpenSeveral,          /// << Message to send to the statusbar when opening several notes of this type.
        OpenOneWith,          /// << Message to send to the statusbar when doing "Open With..." on this note.
        OpenSeveralWith,      /// << Message to send to the statusbar when doing "Open With..." several notes of this type.
        OpenOneWithDialog,    /// << Prompt-message of the "Open With..." dialog for this note.
        OpenSeveralWithDialog /// << Prompt-message of the "Open With..." dialog for several notes of this type.
    };
    virtual QString messageWhenOpening(OpenMessage /*where*/) {
        return QString();
    } /// << @return the message to display according to @p where or nothing if it can't be done. @see OpenMessage describing the nature of the message that should be returned... The default implementation return an empty string. NOTE: If urlToOpen() is invalid and messageWhenOpening() is not empty, then the user will be prompted to edit the note (with the message returned by messageWhenOpening()) for eg. being able to edit URL of a link if it's empty when opening it...
    virtual QString customOpenCommand() {
        return QString();
    }  /// << Reimplement this if your urlToOpen() should be opened with another application instead of the default KDE one. This choice should be left to the users in the setting (choice to use a custom app or not, and which app).
    // Common File Management:                                            ///    (and do save changes) and optionaly hide the toolbar.
    virtual void setFileName(const QString &fileName); /// << Set the filename. Reimplement it if you eg. want to update the view when the filename is changed.
    bool trySetFileName(const QString &fileName);      /// << Set the new filename and return true. Can fail and return false if a file with this fileName already exists.
    QString  fullPath();                               /// << Get the absolute path of the file where this content is stored on disk.
    QString  fileName() const {
        return m_fileName;
    }   /// << Get the file name where this content is stored (relative to the basket folder). @see fullPath().
    qreal      minWidth() const {
        return m_minWidth;
    }    /// << Get the minimum width for this content.
    Note    *note()     {
        return m_note;
    }         /// << Get the note managing this content.
    BasketScene  *basket();                                 /// << Get the basket containing the note managing this content.
    virtual QGraphicsItem *graphicsItem() = 0;
public:
    void setEdited(); /// << Mark the note as edited NOW: change the "last modification time and time" AND save the basket to XML file.
protected:
    void contentChanged(qreal newMinWidth); /// << When the content has changed, inherited classes should call this to specify its new minimum size and trigger a basket relayout.
private:
    Note    *m_note;
    QString  m_fileName;
    qreal    m_minWidth;
public:
    static const int FEEDBACK_DARKING;
};

/** Real implementation of plain text notes:
 * @author Sébastien Laoût
 */
class TextContent : public NoteContent
{
public:
    // Constructor and destructor:
    TextContent(Note *parent, const QString &fileName, bool lazyLoad = false);
    ~TextContent();
    // Simple Generic Methods:
    NoteType::Id type() const;
    QString typeName() const;
    QString lowerTypeName() const;
    QString toText(const QString &/*cuttedFullPath*/);
    QString toHtml(const QString &imageName, const QString &cuttedFullPath);
    bool    useFile() const;
    bool    canBeSavedAs() const;
    QString saveAsFilters() const;
    bool    match(const FilterData &data);
    // Complexe Generic Methods:
    void    exportToHTML(HTMLExporter *exporter, int indent);
    QString cssClass() const;
    qreal     setWidthAndGetHeight(qreal width);
    bool    loadFromFile(bool lazyLoad);
    bool    finishLazyLoad();
    bool    saveToFile();
    QString linkAt(const QPointF &pos);
    void    fontChanged();
    QString editToolTipText() const;
    // Drag and Drop Content:
    QPixmap feedbackPixmap(qreal width, qreal height);
    // Open Content or File:
    QString messageWhenOpening(OpenMessage where);
//  QString customOpenCommand();
    // Content-Specific Methods:
    void    setText(const QString &text, bool lazyLoad = false); /// << Change the text note-content and relayout the note.
    QString text() {
        return m_graphicsTextItem.text();
    }     /// << @return the text note-content.
    QGraphicsItem *graphicsItem() { return &m_graphicsTextItem; }
    
protected:
//     QString          m_text;
    //QTextDocument *m_simpleRichText;
    QGraphicsSimpleTextItem m_graphicsTextItem;
};

#include <QGraphicsTextItem>
/** Real implementation of rich text (HTML) notes:
 * @author Sébastien Laoût
 */
class HtmlContent : public NoteContent
{
public:
    // Constructor and destructor:
    HtmlContent(Note *parent, const QString &fileName, bool lazyLoad = false);
    ~HtmlContent();
    // Simple Generic Methods:
    NoteType::Id type() const;
    QString typeName() const;
    QString lowerTypeName() const;
    QString toText(const QString &/*cuttedFullPath*/);
    QString toHtml(const QString &imageName, const QString &cuttedFullPath);
    bool    useFile() const;
    bool    canBeSavedAs() const;
    QString saveAsFilters() const;
    bool    match(const FilterData &data);
    // Complexe Generic Methods:
    void    exportToHTML(HTMLExporter *exporter, int indent);
    QString cssClass() const;
    qreal     setWidthAndGetHeight(qreal width);
    bool    loadFromFile(bool lazyLoad);
    bool    finishLazyLoad();
    bool    saveToFile();
    QString linkAt(const QPointF &pos);
    void    fontChanged();
    QString editToolTipText() const;
    // Drag and Drop Content:
    QPixmap feedbackPixmap(qreal width, qreal height);
    // Open Content or File:
    QString messageWhenOpening(OpenMessage where);
    QString customOpenCommand();
    // Content-Specific Methods:
    void    setHtml(const QString &html, bool lazyLoad = false); /// << Change the HTML note-content and relayout the note.
    QString html() {
        return m_html;
    }     /// << @return the HTML note-content.
    QGraphicsItem *graphicsItem() { return &m_graphicsTextItem; }
protected:
    QString          m_html;
    QString          m_textEquivalent; //OPTIM_FILTER
    QTextDocument *m_simpleRichText;
    QGraphicsTextItem m_graphicsTextItem;
};

/** Real implementation of image notes:
 * @author Sébastien Laoût
 */
class ImageContent : public NoteContent
{
public:
    // Constructor and destructor:
    ImageContent(Note *parent, const QString &fileName, bool lazyLoad = false);
    ~ImageContent();
    // Simple Generic Methods:
    NoteType::Id type() const;
    QString typeName() const;
    QString lowerTypeName() const;
    QString toHtml(const QString &imageName, const QString &cuttedFullPath);
    QPixmap toPixmap();
    bool    useFile() const;
    bool    canBeSavedAs() const;
    QString saveAsFilters() const;
    bool    match(const FilterData &data);
    // Complexe Generic Methods:
    void    exportToHTML(HTMLExporter *exporter, int indent);
    QString cssClass() const;
    qreal     setWidthAndGetHeight(qreal width);
    bool    loadFromFile(bool lazyLoad);
    bool    finishLazyLoad();
    bool    saveToFile();
    void    fontChanged();
    QString editToolTipText() const;
    void    toolTipInfos(QStringList *keys, QStringList *values);
    // Drag and Drop Content:
    QPixmap feedbackPixmap(qreal width, qreal height);
    bool    needSpaceForFeedbackPixmap() {
        return true;
    }
    // Open Content or File:
    QString messageWhenOpening(OpenMessage where);
    QString customOpenCommand();
    // Content-Specific Methods:
    void    setPixmap(const QPixmap &pixmap); /// << Change the pixmap note-content and relayout the note.
    QPixmap pixmap() {
        return m_pixmapItem.pixmap();
    }     /// << @return the pixmap note-content.
    QByteArray data();
    QGraphicsItem *graphicsItem() { return &m_pixmapItem; }
protected:
    QGraphicsPixmapItem  m_pixmapItem;
    QByteArray m_format;
};

/** Real implementation of animated image (GIF, MNG) notes:
 * @author Sébastien Laoût
 */
class AnimationContent : public QObject, public NoteContent // QObject to be able to receive QMovie signals
{
    Q_OBJECT
public:
    // Constructor and destructor:
    AnimationContent(Note *parent, const QString &fileName, bool lazyLoad = false);
    ~AnimationContent();
    // Simple Generic Methods:
    NoteType::Id type() const;
    QString typeName() const;
    QString lowerTypeName() const;
    QString toHtml(const QString &imageName, const QString &cuttedFullPath);
    QPixmap toPixmap();
    bool    useFile() const;
    bool    canBeSavedAs() const;
    QString saveAsFilters() const;
    bool    match(const FilterData &data);
    void    fontChanged();
    QString editToolTipText() const;
    // Drag and Drop Content:
    QPixmap feedbackPixmap(qreal width, qreal height);
    bool    needSpaceForFeedbackPixmap() {
        return true;
    }
    // Complexe Generic Methods:
    void    exportToHTML(HTMLExporter *exporter, int indent);
    QString cssClass() const;
    qreal     setWidthAndGetHeight(qreal width);
    bool    loadFromFile(bool lazyLoad);
    bool    finishLazyLoad();
    bool    saveToFile();
    // Open Content or File:
    QString messageWhenOpening(OpenMessage where);
    QString customOpenCommand();
    QGraphicsItem *graphicsItem() { return &m_graphicsPixmap; }

    // Content-Specific Methods:
    bool startMovie();

protected slots:
    void movieUpdated();
    void movieResized();
    void movieFrameChanged();

protected:
    QBuffer *m_buffer;
    QMovie  *m_movie;
    qreal m_currentWidth;
    QGraphicsPixmapItem m_graphicsPixmap;
};

/** Real implementation of file notes:
 * @author Sébastien Laoût
 */
class FileContent : public QObject, public NoteContent
{
    Q_OBJECT
public:
    // Constructor and destructor:
    FileContent(Note *parent, const QString &fileName);
    ~FileContent();
    // Simple Generic Methods:
    NoteType::Id type() const;
    QString typeName() const;
    QString lowerTypeName() const;
    QString toHtml(const QString &imageName, const QString &cuttedFullPath);
    bool    useFile() const;
    bool    canBeSavedAs() const;
    QString saveAsFilters() const;
    bool    match(const FilterData &data);
    // Complexe Generic Methods:
    void    exportToHTML(HTMLExporter *exporter, int indent);
    QString cssClass() const;
    qreal     setWidthAndGetHeight(qreal width);
    bool    loadFromFile(bool /*lazyLoad*/);
    void    fontChanged();
    void    linkLookChanged();
    QString editToolTipText() const;
    void    toolTipInfos(QStringList *keys, QStringList *values);
    // Drag and Drop Content:
    QPixmap feedbackPixmap(qreal width, qreal height);
    // Custom Zones:
    int     zoneAt(const QPointF &pos);
    QRectF   zoneRect(int zone, const QPointF &/*pos*/);
    QString zoneTip(int zone);
    Qt::CursorShape cursorFromZone(int zone) const;
    // Content Edition:
    int      xEditorIndent();
    // Open Content or File:
    QString messageWhenOpening(OpenMessage where);
    // Content-Specific Methods:
    void    setFileName(const QString &fileName); /// << Reimplemented to be able to relayout the note.
    virtual LinkLook* linkLook() {
        return LinkLook::fileLook;
    }
    QGraphicsItem *graphicsItem() { return &m_linkDisplayItem; }
protected:
    LinkDisplayItem m_linkDisplayItem;
    // File Preview Management:
protected slots:
    void newPreview(const KFileItem&, const QPixmap &preview);
    void removePreview(const KFileItem&);
    void startFetchingUrlPreview();
protected:
    KIO::PreviewJob *m_previewJob;
};

/** Real implementation of sound notes:
 * @author Sébastien Laoût
 */
class SoundContent : public FileContent // A sound is a file with just a bit different user interaction
{
    Q_OBJECT
public:
    // Constructor and destructor:
    SoundContent(Note *parent, const QString &fileName);
    // Simple Generic Methods:
    NoteType::Id type() const;
    QString typeName() const;
    QString lowerTypeName() const;
    QString toHtml(const QString &imageName, const QString &cuttedFullPath);
    bool    useFile() const;
    bool    canBeSavedAs() const;
    QString saveAsFilters() const;
    bool    match(const FilterData &data);
    QString editToolTipText() const;
    // Complexe Generic Methods:
    QString cssClass() const;
    // Custom Zones:
    QString zoneTip(int zone);
    void    setHoveredZone(int oldZone, int newZone);
    // Open Content or File:
    QString messageWhenOpening(OpenMessage where);
    QString customOpenCommand();
    // Content-Specific Methods:
    LinkLook* linkLook() {
        return LinkLook::soundLook;
    }
    Phonon::MediaObject *music;
private slots:
    void stateChanged(Phonon::State, Phonon::State);

};

/** Real implementation of link notes:
 * @author Sébastien Laoût
 */
class LinkContent : public QObject, public NoteContent
{
    Q_OBJECT
public:
    // Constructor and destructor:
    LinkContent(Note *parent, const QUrl &url, const QString &title, const QString &icon, bool autoTitle, bool autoIcon);
    ~LinkContent();
    // Simple Generic Methods:
    NoteType::Id type() const;
    QString typeName() const;
    QString lowerTypeName() const;
    QString toText(const QString &/*cuttedFullPath*/);
    QString toHtml(const QString &imageName, const QString &cuttedFullPath);
    void    toLink(QUrl *url, QString *title, const QString &cuttedFullPath);
    bool    useFile() const;
    bool    canBeSavedAs() const;
    QString saveAsFilters() const;
    bool    match(const FilterData &data);
    // Complexe Generic Methods:
    void    exportToHTML(HTMLExporter *exporter, int indent);
    QString cssClass() const;
    qreal     setWidthAndGetHeight(qreal width);
    void    saveToNode(QDomDocument &doc, QDomElement &content);
    void    fontChanged();
    void    linkLookChanged();
    QString editToolTipText() const;
    void    toolTipInfos(QStringList *keys, QStringList *values);
    // Drag and Drop Content:
    void    serialize(QDataStream &stream);
    QPixmap feedbackPixmap(qreal width, qreal height);
    // Custom Zones:
    int     zoneAt(const QPointF &pos);
    QRectF   zoneRect(int zone, const QPointF &/*pos*/);
    QString zoneTip(int zone);
    Qt::CursorShape cursorFromZone(int zone) const;
    QString statusBarMessage(int zone);
    // Open Content or File:
    QUrl urlToOpen(bool /*with*/);
    QString messageWhenOpening(OpenMessage where);
    // Content-Specific Methods:
    void    setLink(const QUrl &url, const QString &title, const QString &icon, bool autoTitle, bool autoIcon); /// << Change the link and relayout the note.
    QUrl    url()       {
        return m_url;
    } /// << @return the URL of the link note-content.
    QString title()     {
        return m_title;
    } /// << @return the displayed title of the link note-content.
    QString icon()      {
        return m_icon;
    } /// << @return the displayed icon of the link note-content.
    bool    autoTitle() {
        return m_autoTitle;
    } /// << @return if the title is auto-computed from the URL.
    bool    autoIcon()  {
        return m_autoIcon;
    } /// << @return if the icon is auto-computed from the URL.
    void startFetchingLinkTitle();
    QGraphicsItem *graphicsItem() { return &m_linkDisplayItem; }
protected:
    QUrl        m_url;
    QString     m_title;
    QString     m_icon;
    bool        m_autoTitle;
    bool        m_autoIcon;
    LinkDisplayItem m_linkDisplayItem;
    KIO::Integration::AccessManager*      m_access_manager;
    QNetworkReply*      m_reply;
    QByteArray m_httpBuff; ///< Accumulator for downloaded HTTP data with yet unknown encoding
    bool m_acceptingData; ///< When false, don't accept any HTTP data
    // File Preview Management:
protected slots:
    void httpReadyRead();
    void httpDone(QNetworkReply* reply);
    void newPreview(const KFileItem&, const QPixmap &preview);
    void removePreview(const KFileItem&);
    void startFetchingUrlPreview();
protected:
    KIO::PreviewJob *m_previewJob;
private:
    void decodeHtmlTitle(); ///< Detect encoding of \p m_httpBuff and extract the title from HTML
    void endFetchingLinkTitle(); ///< Extract title and clear http buffer
};

/** Real implementation of cross reference notes:
 * Copied and modified from LinkContent.
 * @author Brian C. Milco
 */
class CrossReferenceContent : public QObject, public NoteContent
{
    Q_OBJECT
public:
    // Constructor and destructor:
    CrossReferenceContent(Note *parent, const QUrl &url, const QString &title, const QString &icon);
    ~CrossReferenceContent();
    // Simple Generic Methods:
    NoteType::Id type() const;
    QString typeName() const;
    QString lowerTypeName() const;
    QString toText(const QString &/*cuttedFullPath*/);
    QString toHtml(const QString &imageName, const QString &cuttedFullPath);
    void    toLink(QUrl *url, QString *title, const QString &cuttedFullPath);
    bool    useFile() const;
    bool    canBeSavedAs() const;
    QString saveAsFilters() const;
    bool    match(const FilterData &data);
    // Complexe Generic Methods:
    void    exportToHTML(HTMLExporter *exporter, int indent);
    QString cssClass() const;
    qreal     setWidthAndGetHeight(qreal);
    void    saveToNode(QDomDocument &doc, QDomElement &content);
    void    fontChanged();
    void    linkLookChanged();
    QString editToolTipText() const;
    void    toolTipInfos(QStringList *keys, QStringList *values);
    // Drag and Drop Content:
    void    serialize(QDataStream &stream);
    QPixmap feedbackPixmap(qreal width, qreal height);
    // Custom Zones:
    int     zoneAt(const QPointF &pos);
    QRectF   zoneRect(int zone, const QPointF &/*pos*/);
    QString zoneTip(int zone);
    Qt::CursorShape cursorFromZone(int zone) const;
    QString statusBarMessage(int zone);
    // Open Content or File:
    QUrl urlToOpen(bool /*with*/);
    QString messageWhenOpening(OpenMessage where);
    // Content-Specific Methods:
    void    setLink(const QUrl &url, const QString &title, const QString &icon); /// << Change the link and relayout the note.
    void    setCrossReference(const QUrl &url, const QString &title, const QString &icon);
    QUrl    url()       {
        return m_url;
    } /// << @return the URL of the link note-content.
    QString title()     {
        return m_title;
    } /// << @return the displayed title of the link note-content.
    QString icon()      {
        return m_icon;
    } /// << @return the displayed icon of the link note-content.

    QGraphicsItem *graphicsItem() { return &m_linkDisplayItem; }

protected:
    QUrl        m_url;
    QString     m_title;
    QString     m_icon;
    LinkDisplayItem m_linkDisplayItem;
};

/** Real implementation of launcher notes:
 * @author Sébastien Laoût
 */
class LauncherContent : public NoteContent
{
public:
    // Constructor and destructor:
    LauncherContent(Note *parent, const QString &fileName);
    ~LauncherContent();
    // Simple Generic Methods:
    NoteType::Id type() const;
    QString typeName() const;
    QString lowerTypeName() const;
    QString toHtml(const QString &imageName, const QString &cuttedFullPath);
    void    toLink(QUrl *url, QString *title, const QString &cuttedFullPath);
    bool    useFile() const;
    bool    canBeSavedAs() const;
    QString saveAsFilters() const;
    bool    match(const FilterData &data);
    // Complexe Generic Methods:
    void    exportToHTML(HTMLExporter *exporter, int indent);
    QString cssClass() const;
    qreal     setWidthAndGetHeight(qreal width);
    bool    loadFromFile(bool /*lazyLoad*/);
    void    fontChanged();
    QString editToolTipText() const;
    void    toolTipInfos(QStringList *keys, QStringList *values);
    // Drag and Drop Content:
    QPixmap feedbackPixmap(qreal width, qreal height);
    // Custom Zones:
    int     zoneAt(const QPointF &pos);
    QRectF   zoneRect(int zone, const QPointF &/*pos*/);
    QString zoneTip(int zone);
    Qt::CursorShape cursorFromZone(int zone) const;
    // Open Content or File:
    QUrl urlToOpen(bool with);
    QString messageWhenOpening(OpenMessage where);
    // Content-Specific Methods:
    void    setLauncher(const QString &name, const QString &icon, const QString &exec); /// << Change the launcher note-content and relayout the note. Normally called by loadFromFile (no save done).
    QString name() {
        return m_name;
    }                              /// << @return the URL of the launcher note-content.
    QString icon() {
        return m_icon;
    }                              /// << @return the displayed icon of the launcher note-content.
    QString exec() {
        return m_exec;
    }                              /// << @return the execute command line of the launcher note-content.
    // TODO: KService *service() ??? And store everything in thta service ?
    
    QGraphicsItem *graphicsItem() { return &m_linkDisplayItem; }

protected:
    QString     m_name; // TODO: Store them in linkDisplay to gain place (idem for Link notes)
    QString     m_icon;
    QString     m_exec;
    LinkDisplayItem m_linkDisplayItem;
};

/** 
 * 
 */
class ColorItem : public QGraphicsItem
{
public:
  ColorItem(Note *parent, const QColor &color);
//   virtual ~ColorItem();
  virtual QColor color() { return m_color; }
  virtual void setColor(const QColor &color);
  
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

private:
  Note *m_note;
  QColor m_color;
  QRectF m_textRect;
  
  static const int RECT_MARGIN;
};

/** Real implementation of color notes:
 * @author Sébastien Laoût
 */
class ColorContent : public NoteContent
{
public:
    // Constructor and destructor:
    ColorContent(Note *parent, const QColor &color);
    virtual ~ColorContent();
    // Simple Generic Methods:
    NoteType::Id type() const;
    QString typeName() const;
    QString lowerTypeName() const;
    QString toText(const QString &/*cuttedFullPath*/);
    QString toHtml(const QString &imageName, const QString &cuttedFullPath);
    bool    useFile() const;
    bool    canBeSavedAs() const;
    QString saveAsFilters() const;
    bool    match(const FilterData &data);
    // Complexe Generic Methods:
    void    exportToHTML(HTMLExporter *exporter, int indent);
    QString cssClass() const;
    qreal     setWidthAndGetHeight(qreal width);
    void    saveToNode(QDomDocument &doc, QDomElement &content);
    void    fontChanged();
    QString editToolTipText() const;
    void    toolTipInfos(QStringList *keys, QStringList *values);
    // Drag and Drop Content:
    void    serialize(QDataStream &stream);
    QPixmap feedbackPixmap(qreal width, qreal height);
    bool    needSpaceForFeedbackPixmap() {
        return true;
    }
    void    addAlternateDragObjects(QMimeData *dragObject);
    // Content-Specific Methods:
    void    setColor(const QColor &color); /// << Change the color note-content and relayout the note.
    QColor  color() {
        return m_colorItem.color();
    }    /// << @return the color note-content.
    QGraphicsItem *graphicsItem() { return &m_colorItem; }
    
protected:
    ColorItem  m_colorItem;
};

/** 
 * 
 */
class UnknownItem : public QGraphicsItem
{
public:
  UnknownItem(Note *parent);
  
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

  virtual QString mimeTypes()
    { return m_mimeTypes; }
  virtual void setMimeTypes(QString mimeTypes);
  virtual void setWidth(qreal width);

private:
  Note *m_note;
  QString m_mimeTypes;
  QRectF m_textRect;
  
  static const qreal DECORATION_MARGIN;
};

/** Real implementation of unknown MIME-types dropped notes:
 * @author Sébastien Laoût
 */
class UnknownContent : public NoteContent
{
public:
    // Constructor and destructor:
    UnknownContent(Note *parent, const QString &fileName);
    ~UnknownContent();
    // Simple Generic Methods:
    NoteType::Id type() const;
    QString typeName() const;
    QString lowerTypeName() const;
    QString toText(const QString &/*cuttedFullPath*/);
    QString toHtml(const QString &imageName, const QString &cuttedFullPath);
    void    toLink(QUrl *url, QString *title, const QString &cuttedFullPath);
    bool    useFile() const;
    bool    canBeSavedAs() const;
    QString saveAsFilters() const;
    bool    match(const FilterData &data);
    // Complexe Generic Methods:
    void    exportToHTML(HTMLExporter *exporter, int indent);
    QString cssClass() const;
    qreal     setWidthAndGetHeight(qreal width);
    bool    loadFromFile(bool /*lazyLoad*/);
    void    fontChanged();
    QString editToolTipText() const;
    // Drag and Drop Content:
    bool    shouldSerializeFile() {
        return false;
    }
    void    addAlternateDragObjects(QMimeData *dragObject);
    QPixmap feedbackPixmap(qreal width, qreal height);
    bool    needSpaceForFeedbackPixmap() {
        return true;
    }
    // Open Content or File:
    QUrl urlToOpen(bool /*with*/) {
        return QUrl();
    }
    
    QGraphicsItem *graphicsItem() { return &m_unknownItem; }
    
    // Content-Specific Methods:
    QString mimeTypes() {
        return m_unknownItem.mimeTypes();
    } /// << @return the list of MIME types this note-content contains.
private:
  UnknownItem m_unknownItem;
};

void NoteFactory__loadNode(const QDomElement &content, const QString &lowerTypeName, Note *parent, bool lazyLoad);

#endif // NOTECONTENT_H
