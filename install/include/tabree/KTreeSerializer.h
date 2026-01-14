// KTreeSerializer.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef KTreeSerializer_h__
#define KTreeSerializer_h__

#include <string>
#include <deque>
#include "KTree.h"
#include "KTreeWalker.h"


namespace tabree {


/**
 * \brief Tree Serialization implemented with KTreeWalker, abstract class for various formats
 */
class KTreeSerializer: public KTreeHandler {
  public:
    KTreeSerializer(std::ostream& Output);
    ~KTreeSerializer() override;
    virtual void SetPrecision(unsigned precision);
    virtual void Serialize(const KTree& Tree, const std::string& Name = "");
  protected:
    std::ostream& fOutput;
    int fPrecision;
};


/**
 * \brief Tree Serialization implemented with KTreeWalker, in KTF format
 */
class KKtfTreeSerializer: public KTreeSerializer {
  public:
    KKtfTreeSerializer(std::ostream& Output);
    ~KKtfTreeSerializer() override;
    bool StartTree(KTree& Node) override ;
    bool StartNode(const std::string& Name, KTree& Node, KTree& AttributeList) override ;
    void EndNode(const std::string& Name, KTree& Node) override ;
    bool StartArray(const std::string& Name, KTree& Node) override ;
  protected:
    char fHeaderChar;
    int fIndentStep, fDepth;
};


/**
 * \brief Tree Serialization implemented with KTreeWalker, in JSON format
 */
class KJsonTreeSerializer: public KTreeSerializer {
  public:
    KJsonTreeSerializer(std::ostream& Output, bool isInline=false);
    ~KJsonTreeSerializer() override;
    bool StartTree(KTree& Node) override ;
  protected:
    virtual void ProcessNode(KTree& Tree, std::string NodeName, const std::string& Indent);
  protected:
    static void WriteJsonValue(std::ostream& os, KVariant& Value, int Precision, int Looseness=0);
    static bool WriteJsonInlineIfSimple(std::ostream& os, KTree& Node, int Precision, int Looseness=0);
    static void WriteJsonString(std::ostream& os, const std::string& Value);
    enum TLooseness {
        Looseness_Strict = 0,
        Looseness_Inline = 1,
        Looseness_Block = 2,
        NumberOfLoosenessLevels
    };
  protected:
    bool fIsInline;
};


/**
 * \brief Tree Serialization implemented with KTreeWalker, in YAML format
 */
class KYamlTreeSerializer: public KJsonTreeSerializer {
  public:
    KYamlTreeSerializer(std::ostream& Output);
    ~KYamlTreeSerializer() override;
    bool StartTree(KTree& Node) override ;
  protected:
    void ProcessYamlObject(KTree& Tree, const std::string& Indent, bool IsArrayElement, bool IsRoot=false);
    void ProcessYamlNode(KTree& Tree, std::string NodeName, const std::string& Indent, const std::string ThisIndent);
};



/**
 * \brief Tree Serialization implemented with KTreeWalker, in XML format
 */
class KXmlTreeSerializer: public KTreeSerializer {
  public:
    KXmlTreeSerializer(std::ostream& Output);
    ~KXmlTreeSerializer() override;
    bool StartTree(KTree& Node) override ;
    bool StartNode(const std::string& Name, KTree& Node, KTree& AttributeList) override ;
    void EndNode(const std::string& Name, KTree& Node) override ;
    bool StartArray(const std::string& Name, KTree& Node) override ;
  protected:
    static void WriteXmlString(std::ostream& os, const std::string& Value);
    static std::string GetXmlNameOf(const std::string& Name);
  protected:
    std::deque<std::string> fIndentStack;
    std::string fNameForEmptyName;
};


/**
 * \brief Tree Serialization implemented with KTreeWalker, in Plist format
 */
class KPlistTreeSerializer: public KTreeSerializer {
  public:
    KPlistTreeSerializer(std::ostream& Output);
    ~KPlistTreeSerializer() override;
    bool StartTree(KTree& Node) override ;
  protected:
    virtual void ProcessNode(KTree& Tree, std::string NodeName, const std::string& Indent);
  protected:
    static void WritePlistString(std::ostream& os, const std::string& Value);
};


/**
 * \brief Tree Serialization implemented with KTreeWalker, in Ini-file format
 */
class KInifileTreeSerializer: public KTreeSerializer {
  public:
    KInifileTreeSerializer(std::ostream& Output);
    ~KInifileTreeSerializer() override;
    bool StartTree(KTree& Node) override;
  protected:
    void ProcessChildren(const std::string& Path, KTree& Node);
    void WriteNode(const std::string& Path, KTree& Node);
};


/**
 * \brief Tree Serialization implemented with KTreeWalker, in XPath-Value Pair format
 */
class KXpvpTreeSerializer: public KTreeSerializer {
  public:
    KXpvpTreeSerializer(std::ostream& Output);
    ~KXpvpTreeSerializer() override;
    bool StartNode(const std::string& Name, KTree& Node, KTree& AttributeList) override;
};

    

class KTreeOutputStream {
  public:
    KTreeOutputStream(std::ostream& os, const std::string& format): os(os), format(format) {}
  protected:
    std::ostream& os;
    std::string format;
    friend std::ostream& operator<<(KTreeOutputStream tos, const KTree& tree) {
        if (tos.format == "json") {
            KJsonTreeSerializer(tos.os).Serialize(tree);
        }
        else if (tos.format == "json-inline") {
            KJsonTreeSerializer(tos.os, true).Serialize(tree);
        }
        else if (tos.format == "yaml") {
            KYamlTreeSerializer(tos.os).Serialize(tree);
        }
        else if (tos.format == "xml") {
            KXmlTreeSerializer(tos.os).Serialize(tree);
        }
        else if (tos.format == "plist") {
            KPlistTreeSerializer(tos.os).Serialize(tree);
        }
        else if (tos.format == "ini") {
            KInifileTreeSerializer(tos.os).Serialize(tree);
        }
        else if (tos.format == "ktf") { // original format, YAML-like
            KKtfTreeSerializer(tos.os).Serialize(tree);
        }
        else if (tos.format == "xpvp") { // original format, list of XPath-Value pair
            KXpvpTreeSerializer(tos.os).Serialize(tree);
        }
        else {
            KKtfTreeSerializer(tos.os).Serialize(tree);
        }
        return tos.os;
    }
};


class setformat {
  public:
    setformat(const std::string& format): format(format) {}
  protected:
    std::string format;
    friend KTreeOutputStream operator<<(std::ostream& os, const setformat& fmt) {
        return KTreeOutputStream(os, fmt.format);
    }
};

}
#endif
