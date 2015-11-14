/*
* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "SelectorQueryResultsCache.h"

#include "Node.h"
#include "Element.h"
#include "core/dom/StaticNodeList.h"

#include <wtf/text/AtomicString.h>
/*
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
*/

namespace blink {

SelectorQueryResultsCache* SelectorQueryResultsCache::getInstance() {
    static SelectorQueryResultsCache *instance = 0;
    if (!instance)
        instance = new SelectorQueryResultsCache();
    return instance;
}

SelectorQueryResultsCache::SelectorQueryResultsCache() {
    for (int i = 0; i < SELECTOR_QUERY_RESULTS_CACHE_SIZE; i++){
        items[i] = NULL;
    }
    m_cacheHasElements = false;
}

SelectorQueryResultsCache::~SelectorQueryResultsCache() {
    invalidateAll();
    for (int i = 0; i < SELECTOR_QUERY_RESULTS_CACHE_SIZE; i++){
        if (items[i]) {
            delete items[i];
        }
    }
}

PassRefPtrWillBeRawPtr<StaticElementList> SelectorQueryResultsCache::addQuery(const Node *node, const AtomicString& selectors, PassRefPtrWillBeRawPtr<StaticElementList> nodeList) {
    unsigned int hashIdx = SelectorQueryResultsCache::getHashIdx(node, selectors);
    SelectorQueryResultsCache::Item *item = items[hashIdx];

    if (!item)
        item = items[hashIdx] = new Item;
    item->node = node;
    item->selectors = selectors.string();
    item->nodeList = RefPtr<StaticElementList>(nodeList);

    RefPtr<StaticElementList> tmp = item->nodeList;
    m_cacheHasElements = true;
    return tmp.release();
}

PassRefPtrWillBeRawPtr<StaticElementList> SelectorQueryResultsCache::getQuery(const Node *node, const AtomicString& selectors) {
    //Caller must guarantee node is non-null
    unsigned int hashIdx = SelectorQueryResultsCache::getHashIdx(node, selectors);
    SelectorQueryResultsCache::Item *item = items[hashIdx];

    if (!item || !node->isSameNode(item->node) || (item->selectors != selectors.string()))
        return nullptr;
    RefPtr<StaticElementList> tmp = item->nodeList;
    return tmp.release();
}

void SelectorQueryResultsCache::invalidateAll() {
    if (m_cacheHasElements) {
        for (int i = 0; i < SELECTOR_QUERY_RESULTS_CACHE_SIZE; i++) {
            SelectorQueryResultsCache::Item *item = items[i];
            if (item) {
                item->node = nullptr;
                item->nodeList = nullptr;
            }
        }
    }
    m_cacheHasElements = false;
}

unsigned int SelectorQueryResultsCache::getHashIdx(const Node *node, const AtomicString& selectors) {
    unsigned int hashKey = 0;
    const String s = selectors.string();
    for (unsigned int i = 0; i < s.length(); i++)
        hashKey = hashKey * 31 + s[i];
    return hashKey & (SELECTOR_QUERY_RESULTS_CACHE_SIZE - 1); //faster than modulo
    //TODO hash hashKey with node address >> something based on the size of node to get the significant bits into [0:8]
}

} //namespace
