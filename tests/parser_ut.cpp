#include <QtTest>
#include <QFile>
#include <QSet>
#include "../libircclient/parser.h"
#include "../libirc/irc_numerics.h"

class ParserTest : public QObject
{
        Q_OBJECT

    private Q_SLOTS:
        void testCasePrivmsg();
        void testCasePingWithoutPrefix();
        void testCaseIrcv3ServerTime();
        void testCaseIrcv3MultipleTags();
        void testCaseIrcv3EscapedTagValues();
        void testCaseIrcv3InvalidTagOnlyLine();
        void testCaseServerCommunicationFixture();
};

void ParserTest::testCasePrivmsg()
{
    libircclient::Parser parser(":nick!ident@example.org PRIVMSG #grumpy :hello world");

    QVERIFY(parser.IsValid());
    QCOMPARE(parser.GetNumeric(), IRC_NUMERIC_RAW_PRIVMSG);
    QCOMPARE(parser.GetSourceInfo(), QString("nick!ident@example.org"));
    QVERIFY(parser.GetSourceUserInfo() != nullptr);
    QCOMPARE(parser.GetSourceUserInfo()->GetNick(), QString("nick"));
    QCOMPARE(parser.GetSourceUserInfo()->GetIdent(), QString("ident"));
    QCOMPARE(parser.GetSourceUserInfo()->GetHost(), QString("example.org"));
    QCOMPARE(parser.GetParameters(), QList<QString>() << "#grumpy");
    QCOMPARE(parser.GetText(), QString("hello world"));
    QCOMPARE(parser.GetRaw(), QString(":nick!ident@example.org PRIVMSG #grumpy :hello world"));
    QCOMPARE(parser.GetOriginalRaw(), QString(":nick!ident@example.org PRIVMSG #grumpy :hello world"));
}

void ParserTest::testCasePingWithoutPrefix()
{
    libircclient::Parser parser("PING :server.example.org");

    QVERIFY(parser.IsValid());
    QCOMPARE(parser.GetNumeric(), IRC_NUMERIC_RAW_PING);
    QCOMPARE(parser.GetParameterLine(), QString("server.example.org"));
    QCOMPARE(parser.GetParameters(), QList<QString>() << "server.example.org");
}

void ParserTest::testCaseIrcv3ServerTime()
{
    libircclient::Parser parser("@time=2011-10-19T16:40:51.620Z :Angel!angel@example.org PRIVMSG Wiz :Hello");

    QVERIFY(parser.IsValid());
    QCOMPARE(parser.GetNumeric(), IRC_NUMERIC_RAW_PRIVMSG);
    QCOMPARE(parser.GetRaw(), QString(":Angel!angel@example.org PRIVMSG Wiz :Hello"));
    QCOMPARE(parser.GetOriginalRaw(), QString("@time=2011-10-19T16:40:51.620Z :Angel!angel@example.org PRIVMSG Wiz :Hello"));
    QCOMPARE(parser.GetTag("time"), QString("2011-10-19T16:40:51.620Z"));
    QVERIFY(parser.GetTimestamp().isValid());
    QCOMPARE(parser.GetTimestamp().date(), QDate(2011, 10, 19));
    QCOMPARE(parser.GetTimestamp().time().hour(), 16);
    QCOMPARE(parser.GetTimestamp().time().minute(), 40);
    QCOMPARE(parser.GetTimestamp().time().second(), 51);
    QCOMPARE(parser.GetTimestamp().time().msec(), 620);
}

void ParserTest::testCaseIrcv3MultipleTags()
{
    libircclient::Parser parser("@account=alice;away;example.com/key=value :nick!u@h JOIN #grumpy :Alice Doe");

    QVERIFY(parser.IsValid());
    QCOMPARE(parser.GetNumeric(), IRC_NUMERIC_RAW_JOIN);
    QCOMPARE(parser.GetTag("account"), QString("alice"));
    QCOMPARE(parser.GetTag("away"), QString(""));
    QCOMPARE(parser.GetTag("example.com/key"), QString("value"));
    QCOMPARE(parser.GetParameters(), QList<QString>() << "#grumpy");
    QCOMPARE(parser.GetText(), QString("Alice Doe"));
}

void ParserTest::testCaseIrcv3EscapedTagValues()
{
    libircclient::Parser parser("@label=hello\\sworld\\:\\\\ :nick!u@h NOTICE #grumpy :test");

    QVERIFY(parser.IsValid());
    QCOMPARE(parser.GetNumeric(), IRC_NUMERIC_RAW_NOTICE);
    QCOMPARE(parser.GetTag("label"), QString("hello world;\\"));
}

void ParserTest::testCaseIrcv3InvalidTagOnlyLine()
{
    libircclient::Parser parser("@time=2011-10-19T16:40:51.620Z");

    QVERIFY(!parser.IsValid());
    QCOMPARE(parser.GetOriginalRaw(), QString("@time=2011-10-19T16:40:51.620Z"));
}

void ParserTest::testCaseServerCommunicationFixture()
{
    QFile fixture(":/fixtures/fixtures/tm_irc_login.txt");
    QVERIFY2(fixture.open(QIODevice::ReadOnly), "fixture must be embedded");

    const QString input = QString::fromUtf8(fixture.readAll());
    const QStringList lines = input.split('\n');
    int inbound_count = 0;
    int outbound_count = 0;
    int valid_inbound_count = 0;
    int invalid_inbound_count = 0;
    int tagged_count = 0;
    QHash<int, int> numeric_counts;
    QSet<QString> supported_caps;
    QSet<QString> acked_caps;
    bool saw_ping = false;
    bool saw_cap_ls = false;
    bool saw_cap_ack = false;
    bool saw_welcome = false;
    bool saw_myinfo = false;
    bool saw_isupport_prefix = false;
    bool saw_isupport_chanmodes = false;
    bool saw_displayed_host = false;
    bool saw_user_mode = false;
    bool saw_extended_join = false;
    bool saw_channel_mode = false;
    bool saw_names = false;
    bool saw_end_of_names = false;
    bool saw_mode_info = false;
    bool saw_creation_time = false;
    bool saw_end_of_bans = false;

    foreach (QString line, lines)
    {
        line = line.trimmed();
        if (line.isEmpty())
            continue;

        QVERIFY2(line.contains('|'), qPrintable("fixture line missing direction: " + line));
        const QString direction = line.mid(0, line.indexOf('|'));
        const QString raw = line.mid(line.indexOf('|') + 1);

        if (direction == "out")
        {
            outbound_count++;
            continue;
        }

        QCOMPARE(direction, QString("in"));
        inbound_count++;

        libircclient::Parser parser(raw);
        if (!parser.IsValid())
        {
            invalid_inbound_count++;
            continue;
        }

        valid_inbound_count++;
        numeric_counts[parser.GetNumeric()]++;

        if (!parser.GetTags().isEmpty())
            tagged_count++;

        switch (parser.GetNumeric())
        {
            case IRC_NUMERIC_RAW_PING:
                saw_ping = true;
                QCOMPARE(parser.GetParameters(), QList<QString>() << "6071ADAB");
                break;
            case IRC_NUMERIC_RAW_CAP:
                if (parser.GetParameters().size() >= 2 && parser.GetParameters()[1] == "LS")
                {
                    saw_cap_ls = true;
                    foreach (QString cap, parser.GetText().split(' ', Qt::SkipEmptyParts))
                        supported_caps.insert(cap);
                }
                if (parser.GetParameters().size() >= 2 && parser.GetParameters()[1] == "ACK")
                {
                    saw_cap_ack = true;
                    foreach (QString cap, parser.GetText().split(' ', Qt::SkipEmptyParts))
                        acked_caps.insert(cap);
                }
                break;
            case IRC_NUMERIC_WELCOME:
                saw_welcome = true;
                QCOMPARE(parser.GetParameters(), QList<QString>() << "TestUser");
                QVERIFY(parser.GetText().contains("TestUser!tester@example-host.invalid"));
                break;
            case IRC_NUMERIC_MYINFO:
                saw_myinfo = true;
                QVERIFY(parser.GetParameters().contains("UnrealIRCd-6.1.9.1"));
                break;
            case IRC_NUMERIC_ISUPPORT:
                saw_isupport_prefix = saw_isupport_prefix || parser.GetParameters().contains("PREFIX=(qaohv)~&@%+");
                saw_isupport_chanmodes = saw_isupport_chanmodes || parser.GetParameters().filter("CHANMODES=").size() == 1;
                break;
            case 396:
                saw_displayed_host = true;
                QCOMPARE(parser.GetParameters(), QList<QString>() << "TestUser" << "hidden-host.example.net");
                break;
            case IRC_NUMERIC_RAW_MODE:
                if (parser.GetParameters().size() >= 1 && parser.GetParameters()[0] == "TestUser")
                {
                    saw_user_mode = true;
                    QCOMPARE(parser.GetText(), QString("+iwxz"));
                }
                if (parser.GetParameters().size() >= 2 && parser.GetParameters()[0] == "#testchannel")
                {
                    saw_channel_mode = true;
                    QCOMPARE(parser.GetParameters()[1], QString("+nt"));
                    QCOMPARE(parser.GetTag("time"), QString("2026-08-08T17:05:47.771Z"));
                }
                break;
            case IRC_NUMERIC_RAW_JOIN:
                saw_extended_join = true;
                QCOMPARE(parser.GetParameters(), QList<QString>() << "#testchannel" << "*");
                QCOMPARE(parser.GetText(), QString("Test User"));
                QCOMPARE(parser.GetTag("time"), QString("2026-08-08T17:05:47.771Z"));
                QVERIFY(parser.GetTimestamp().isValid());
                QCOMPARE(parser.GetTimestamp().date(), QDate(2026, 8, 8));
                break;
            case IRC_NUMERIC_NAMREPLY:
                saw_names = true;
                QCOMPARE(parser.GetParameters(), QList<QString>() << "TestUser" << "=" << "#testchannel");
                QCOMPARE(parser.GetText(), QString("@TestUser"));
                break;
            case IRC_NUMERIC_ENDOFNAMES:
                saw_end_of_names = true;
                QCOMPARE(parser.GetParameters(), QList<QString>() << "TestUser" << "#testchannel");
                break;
            case IRC_NUMERIC_MODEINFO:
                saw_mode_info = true;
                QCOMPARE(parser.GetParameters(), QList<QString>() << "TestUser" << "#testchannel" << "+nt");
                break;
            case IRC_NUMERIC_CREATIONTIME:
                saw_creation_time = true;
                QCOMPARE(parser.GetParameters(), QList<QString>() << "TestUser" << "#testchannel" << "1786208747");
                break;
            case IRC_NUMERIC_ENDOFBANS:
                saw_end_of_bans = true;
                QCOMPARE(parser.GetParameters(), QList<QString>() << "TestUser" << "#testchannel");
                break;
        }
    }

    QCOMPARE(outbound_count, 9);
    QCOMPARE(inbound_count, 36);
    QCOMPARE(valid_inbound_count, inbound_count);
    QCOMPARE(invalid_inbound_count, 0);
    QCOMPARE(tagged_count, 2);
    QVERIFY(saw_ping);
    QVERIFY(saw_cap_ls);
    QVERIFY(saw_cap_ack);
    QVERIFY(saw_welcome);
    QVERIFY(saw_myinfo);
    QVERIFY(saw_isupport_prefix);
    QVERIFY(saw_isupport_chanmodes);
    QVERIFY(saw_displayed_host);
    QVERIFY(saw_user_mode);
    QVERIFY(saw_extended_join);
    QVERIFY(saw_channel_mode);
    QVERIFY(saw_names);
    QVERIFY(saw_end_of_names);
    QVERIFY(saw_mode_info);
    QVERIFY(saw_creation_time);
    QVERIFY(saw_end_of_bans);
    QVERIFY(supported_caps.contains("away-notify"));
    QVERIFY(supported_caps.contains("extended-join"));
    QVERIFY(supported_caps.contains("multi-prefix"));
    QVERIFY(supported_caps.contains("chghost"));
    QVERIFY(supported_caps.contains("server-time"));
    QCOMPARE(acked_caps, QSet<QString>() << "away-notify" << "extended-join" << "multi-prefix" << "chghost" << "server-time");
    QCOMPARE(numeric_counts.value(IRC_NUMERIC_MOTD), 5);
    QCOMPARE(numeric_counts.value(IRC_NUMERIC_ISUPPORT), 4);
}

QTEST_APPLESS_MAIN(ParserTest)

#include "parser_ut.moc"
