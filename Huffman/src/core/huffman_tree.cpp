#include "huffman/core/huffman_tree.hpp"

#include <cstdint>
#include <queue>
#include <vector>

namespace huffman::core
{
    namespace
    {
        struct Node
        {
            std::uint64_t weight;
            int left;
            int right;
            Byte symbol;
        };

        void assignDepths(const std::vector<Node> &nodes, int index, CodeLength depth, CodeLengths &lengths)
        {
            const Node &node = nodes[static_cast<std::size_t>(index)];

            if (node.left == -1 && node.right == -1)
            {
                lengths[node.symbol] = depth;
                return;
            }

            assignDepths(nodes, node.left, static_cast<CodeLength>(depth + 1), lengths);
            assignDepths(nodes, node.right, static_cast<CodeLength>(depth + 1), lengths);
        };

        std::uint64_t kraftUnits(const CodeLengths &lengths, int maxCodeLength)
        {
            std::uint64_t units = 0;

            for (CodeLength length : lengths)
            {
                if (length > 0)
                {
                    units += (std::uint64_t{1} << (maxCodeLength - length));
                }
            }

            return units;
        };

        void limitCodeLengths(CodeLengths &lengths)
        {
            constexpr int maxCodeLength = HuffmanTree::kMaxCodeLength;

            for (CodeLength &length : lengths)
                if (length > maxCodeLength)
                    length = static_cast<CodeLength>(maxCodeLength);

            const std::uint64_t limit = std::uint64_t{1} << maxCodeLength;

            while (kraftUnits(lengths, maxCodeLength) > limit)
            {
                int maxAllowedLength = maxCodeLength - 1;
                while (maxAllowedLength >= 1)
                {
                    bool found = false;
                    for (std::size_t i = 0; i < kAlphabetSize; ++i)
                    {
                        if (lengths[i] == maxAllowedLength)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        break;
                    --maxAllowedLength;
                }

                for (std::size_t i = 0; i < kAlphabetSize; ++i)
                {
                    if (lengths[i] == static_cast<CodeLength>(maxAllowedLength))
                    {
                        lengths[i] = static_cast<CodeLength>(maxAllowedLength + 1);
                        break;
                    }
                }
            }
        }

    }

    CodeLengths HuffmanTree::buildHuffmanCodeLengths(const FrequencyTable &frequencyTable)
    {
        CodeLengths lengths{};

        const std::size_t distinctSymbols = frequencyTable.distinctSymbols();

        if (distinctSymbols == 0)
            return lengths;

        if (distinctSymbols == 1)
        {
            for (std::size_t i = 0; i < kAlphabetSize; ++i)
                if (frequencyTable.getFrequency(static_cast<Byte>(i)) > 0)
                    lengths[i] = 1;
            return lengths;
        }

        std::vector<Node> nodes;
        nodes.reserve(distinctSymbols * 2);

        auto cmp = [&nodes](int left, int right)
        {
            const auto &nLeft = nodes[static_cast<std::size_t>(left)];
            const auto &nRight = nodes[static_cast<std::size_t>(right)];

            if (nLeft.weight != nRight.weight)
                return nLeft.weight > nRight.weight;
            return left > right;
        };

        std::priority_queue<int, std::vector<int>, decltype(cmp)> heap(cmp);

        for (std::size_t i = 0; i < kAlphabetSize; ++i)
        {
            const std::uint64_t freq = frequencyTable.getFrequency(static_cast<Byte>(i));
            if (freq > 0)
            {
                nodes.push_back(Node{freq, -1, -1, static_cast<Byte>(i)});
                heap.push(static_cast<int>(nodes.size() - 1));
            }
        }

        while (heap.size() > 1)
        {
            const int leftIndex = heap.top();
            heap.pop();
            const int rightIndex = heap.top();
            heap.pop();

            const std::uint64_t combinedWeight = nodes[static_cast<std::size_t>(leftIndex)].weight +
                                                 nodes[static_cast<std::size_t>(rightIndex)].weight;

            nodes.push_back(Node{combinedWeight, leftIndex, rightIndex, 0});
            heap.push(static_cast<int>(nodes.size() - 1));
        }

        assignDepths(nodes, heap.top(), 0, lengths);
        limitCodeLengths(lengths);
        return lengths;
    }

}
