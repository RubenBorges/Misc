#include <list>
#include <vector>
#include <stdexcept>

#include <iostream>

template <typename T>
class indexed_list {
public:
    using list_type      = std::list<T>;
    using iterator       = typename list_type::iterator;
    using const_iterator = typename list_type::const_iterator;

    // Insert at end
    iterator push_back(const T& value) {
        data_.push_back(value);
        auto it = std::prev(data_.end());
        index_.push_back(it);
        return it;
    }

    iterator push_back(T&& value) {
        data_.push_back(std::move(value));
        auto it = std::prev(data_.end());
        index_.push_back(it);
        return it;
    }

    // Insert at specific index (random-access insert)
    iterator insert(std::size_t pos, const T& value) {
        if (pos > index_.size())
            throw std::out_of_range("indexed_list::insert");

        auto it = data_.insert(pos == index_.size() ? data_.end()
                                                    : index_[pos],
                               value);

        index_.insert(index_.begin() + pos, it);
        return it;
    }

    // Erase by iterator
    void erase(iterator it) {
        // Find index position
        for (std::size_t i = 0; i < index_.size(); ++i) {
            if (index_[i] == it) {
                index_.erase(index_.begin() + i);
                data_.erase(it);
                return;
            }
        }
    }

    // Erase by index
    void erase(std::size_t pos) {
        if (pos >= index_.size())
            throw std::out_of_range("indexed_list::erase");

        auto it = index_[pos];
        index_.erase(index_.begin() + pos);
        data_.erase(it);
    }

    // Random access
    T& operator[](std::size_t pos) {
        return *index_[pos];
    }

    const T& operator[](std::size_t pos) const {
        return *index_[pos];
    }

    std::size_t size() const noexcept {
        return index_.size();
    }

    bool empty() const noexcept {
        return index_.empty();
    }

    iterator begin() noexcept { return data_.begin(); }
    iterator end()   noexcept { return data_.end();   }

    const_iterator begin() const noexcept { return data_.begin(); }
    const_iterator end()   const noexcept { return data_.end();   }

private:
    list_type data_;              // stable nodes
    std::vector<iterator> index_; // random-access index
};


/*---------------\
| Usage Example  |
\---------------*/

int main() {
    indexed_list<std::string> il;

    il.push_back("alpha");
    il.push_back("beta");
    il.push_back("gamma");

    il.insert(1, "inserted");

    std::cout << il[0] << "\n"; // alpha
    std::cout << il[1] << "\n"; // inserted
    std::cout << il[2] << "\n"; // beta
std::cout << il[3] << "\n"; // beta
    il.erase(1);

    std::cout << il[1] << "\n"; // beta
    std::cout << il[2] << "\n"; // beta
}
   
