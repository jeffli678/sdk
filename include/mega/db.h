/**
 * @file mega/db.h
 * @brief Database access interface
 *
 * (c) 2013-2014 by Mega Limited, Auckland, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#ifndef MEGA_DB_H
#define MEGA_DB_H 1

#include "filesystem.h"

namespace mega {
// generic host transactional database access interface
class DBTableTransactionCommitter;

class MEGA_API DbTable
{
    static const int IDSPACING = 16;
    PrnGen &rng;

protected:
    bool mCheckAlwaysTransacted = false;
    DBTableTransactionCommitter* mCurrentTransactionCommiter = nullptr;
    friend class DBTableTransactionCommitter;
    void checkTransaction();

public:
    // for a full sequential get: rewind to first record
    virtual void rewind() = 0;

    // get next record in sequence
    virtual bool next(uint32_t*, string*) = 0;
    bool next(uint32_t*, string*, SymmCipher*);

    // get specific record by key
    virtual bool get(uint32_t, string*) = 0;

    // update or add specific record
    virtual bool put(uint32_t, char*, unsigned) = 0;
    bool put(uint32_t, string*);
    bool put(uint32_t, Cacheable *, SymmCipher*);

    // delete specific record
    virtual bool del(uint32_t) = 0;

    // delete all records
    virtual void truncate() = 0;

    // begin transaction
    virtual void begin() = 0;

    // commit transaction
    virtual void commit() = 0;

    // abort transaction
    virtual void abort() = 0;

    // permanantly remove all database info
    virtual void remove() = 0;

    void checkCommitter(DBTableTransactionCommitter*);

    // autoincrement
    uint32_t nextid;

    DbTable(PrnGen &rng, bool alwaysTransacted);
    virtual ~DbTable() { }
};

class MEGA_API DBTableTransactionCommitter
{
    DbTable* mTable;
    bool mStarted = false;

public:
    inline void beginOnce()
    {
        if (mTable && !mStarted)
        {
            mTable->begin();
            mStarted = true;
        }
    }

    inline ~DBTableTransactionCommitter()
    {
        if (mTable)
        {
            if (mStarted)
            {
                mTable->commit();
            }
            mTable->mCurrentTransactionCommiter = nullptr;
        }
    }

    explicit inline DBTableTransactionCommitter(DbTable* t)
        : mTable(t)
    {
        if (mTable)
        {
            if (mTable->mCurrentTransactionCommiter)
            {
                mTable = nullptr;  // we are nested; this one does nothing.  This can occur during eg. putnodes response when the core sdk and the intermediate layer both do db work.
            }
            else
            {
                mTable->mCurrentTransactionCommiter = this;
            }
        }
    }
};

struct MEGA_API DbAccess
{
    static const int LEGACY_DB_VERSION = 11;
    static const int DB_VERSION = LEGACY_DB_VERSION + 1;

    DbAccess();
    virtual DbTable* open(PrnGen &rng, FileSystemAccess*, string*, bool recycleLegacyDB, bool checkAlwaysTransacted) = 0;

    virtual ~DbAccess() { }

    int currentDbVersion;
};
} // namespace

#endif
